#include "ui.h"
#include <map>
#include <ranges>
#include <algorithm>
#include "pick_window.h"
#include "config_window.h"
#include "browser.h"
#include "discovery.h"
#include "grey.h"
#include "config.h"
#include "../globals.h"
#include "win32/npipe.h"
#include "ext/alg_tracker.h"
#include <fmt/core.h>
#include "ext/alg_tracker.h"
#include "str.h"
#include "fss.h"
#include "win32/shell.h"
#include "win32/process.h"
#include "win32/kernel.h"
#include "win32/user.h"

namespace bt::ui {

    alg::tracker t{SysAppName, APP_VERSION};
    unique_ptr<grey::backend> active_backend;
    std::function<void(bool is_open)> on_ui_open_changed;
    bool is_main_instance{false};
    bool is_config_running{false};

    void set_main_instance() {
        is_main_instance = true;
        t.add_constant("iid", config::i.get_iid());

        t.track(map<string, string>
        {
            { "event", "start" },
            { "dir", fss::get_current_dir() }
        }, true);
    }

    void flush() {
        t.track(map<string, string>
        {
            { "event", "ping" }
        }, true);
    }

    void autoshutdown(size_t inactive_mins) {
        t.track(map<string, string> {
            { "event", "autoshutdown" },
            { "inactive_mins", std::to_string(inactive_mins)}
        }, false);

        flush();
    }

    void render_ui_frame_if_required() {
        if(!active_backend) return;

        active_backend->run_one_frame();
        active_backend->post_run_one_frame();

        // autodestroy backend if no windows are running
        if(active_backend->window_count() == 0) {
            active_backend.reset();

            if(on_ui_open_changed) {
                on_ui_open_changed(false);
            }
        }
    }

    void prepare_ui_backend() {
        if(active_backend) return;

        if(on_ui_open_changed) {
            on_ui_open_changed(true);
        }

        string title = fmt::format("{} {}", AppName, APP_VERSION);
        active_backend = grey::backend::make_platform_default(title);

        string theme_id = config::i.get_theme();
        if(!theme_id.empty()) {
            active_backend->set_theme(theme_id);
        }
    }

    void picker(url_payload& up, std::vector<shared_ptr<browser_instance>> choices) {
        prepare_ui_backend();

        float scale = active_backend->get_system_scale();
        float height = min(400, (choices.size() + 1) * 50);
        auto w = active_backend->make_window<pick_window>(up, 300, height, choices);
        w->can_resize = false;
        w->center();
        w->bring_to_top();
        w->detach_on_close = true;
    }

    void url_open(url_payload up, open_method method) {

        // read in method if required
        if(method == open_method::configured) {
            auto sm = config::i.get_open_method();
            if(sm == "silent") {
                method = open_method::silent;
            } else if(sm == "pick") {
                method = open_method::pick;
            } else {
                method = open_method::decide;
            }
        }

        if(method == open_method::pick) {
            up.method = "pick";
            picker(up, browser::to_instances(config::i.load_browsers()));
        } else {
            // silent or decide
            string real_url;
            auto matches = browser::match(config::i.load_browsers(), up.url, real_url);
            up.url = real_url;
            browser_match_result& first_match = matches[0];

            if(method == open_method::silent) {
                up.method = "silent";
                first_match.bi->launch(up);

                app_event(
                    "rule_open", 
                    up.url, 
                    fmt::format("{} ({}), rule: {}",
                        first_match.bi->b->name,
                        first_match.bi->name,
                        first_match.rule.value));

            } else if(matches.size() == 1) {
                up.method = "decide_single_match";
                first_match.bi->launch(up);

                app_event(
                    "rule_open", 
                    up.url, 
                    fmt::format("{} ({}), rule: {}",
                        first_match.bi->b->name,
                        first_match.bi->name,
                        first_match.rule.value));

            } else {
                // force pick for the remaining instances
                vector<shared_ptr<browser_instance>> choices;
                for(const auto& bmr : matches) choices.push_back(bmr.bi);
                picker(up, choices);
            }
        }
    }

    void config() {
        // allow only single instance
        if(is_config_running) return;

        prepare_ui_backend();

        float scale = active_backend->get_system_scale();
        auto w = active_backend->make_window<config_window>();
        w->detach_on_close = true;
        w->on_open_changed = [w](bool& is_open) {
            is_config_running = is_open;
        };
        is_config_running = true;
    }

    void ensure_no_instances() {
        //normal shutdown
        try_invoke_running_instance("shutdown");

        //legacy shutdown for v2.5.2 and earlier
        win32::npipe pipe(AppGuid);
        pipe.send("x");
    }

    // Tries to find and invoke a running instance. If succeeded, returns true
    bool try_invoke_running_instance(const string& data) {
        if(is_main_instance) return false; // don't send to self
        HWND hwnd = ::FindWindow(str::to_wstr(Win32ClassName).c_str(), str::to_wstr(AppGuid).c_str());
        if(!hwnd) return false;

        COPYDATASTRUCT cds;
        cds.dwData = '?';
        cds.cbData = data.size() + 1;
        cds.lpData = (LPVOID)data.c_str();

        // SendMessaeg shoudl return true if app processes the message, false otherwise
        return ::SendMessage(hwnd, WM_COPYDATA, (WPARAM)hwnd, (LPARAM)(LPVOID)&cds);
    }

    void contact() {
        url_open(
            url_payload{HomeUrl + "#contact"},
            ui::open_method::configured);

        //string body = fmt::format("Hi there!%0A insert your question here%0A%0A System Information%0AVersion: {}%0A", Version);
        //win32::shell::exec(fmt::format("mailto:rznel9ynn@mozmail.com?subject=Browser Tamer&body={}", body), "");
    }

    void coffee(const string& from) {
        ui::url_open(url_payload{CoffeePageUrl, "", "ui_coffee"}, ui::open_method::silent);

        t.track(map<string, string>
        {
            { "event", "give_coffee" },
            { "from", from }
        }, false);
    }

    bool is_picker_hotkey_down() {
        string hk = config::i.get_picker_hotkey();

        if(hk == "cs") return win32::user::is_kbd_ctrl_down() && win32::user::is_kbd_shift_down();
        if(hk == "ca") return win32::user::is_kbd_ctrl_down() && win32::user::is_kbd_alt_down();
        if(hk == "as") return win32::user::is_kbd_alt_down() && win32::user::is_kbd_shift_down();

        return false;
    }
}