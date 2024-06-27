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
#include <fmt/core.h>
#include "str.h"
#include "fss.h"
#include "win32/shell.h"
#include "win32/process.h"
#include "win32/kernel.h"
#include "win32/user.h"
#include "win32/monitor.h"

namespace bt::ui {

    unique_ptr<grey::backend> active_backend;
    std::function<void(bool is_open)> on_ui_open_changed;
    bool is_main_instance{false};
    bool is_config_running{false};
    bool is_url_pipeline_running{false};

    void set_main_instance() {
        is_main_instance = true;

        t.add_constant("iid", g_config.get_iid());
        t.track(map<string, string> {
            { "event", "start" }
        }, true);
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

        string title = fmt::format("{} {}", APP_LONG_NAME, APP_VERSION);
        active_backend = grey::backend::make_platform_default(title);

        if(!g_config.theme_id.empty()) {
            active_backend->set_theme(g_config.theme_id);
        }
    }

    void picker(url_payload& up, std::vector<shared_ptr<browser_instance>> choices) {
        prepare_ui_backend();

        float scale = active_backend->get_system_scale();
        float height = min(400, (choices.size() + 1) * 50);
        auto w = active_backend->make_window<pick_window>(up, 300, height, choices);
        w->can_resize = false;
        void* h_monitor{nullptr};
        if(up.source_window_handle) {
            win32::monitor monitor = win32::monitor::get_nearest(up.source_window_handle);
            h_monitor = static_cast<void*>(monitor.h);
        }
        w->center(h_monitor);
        w->bring_to_top();
        w->detach_on_close = true;
    }

    void url_open(url_payload up, open_method method) {

        g_pipeline.process(up);

        // read in method if required
        if(method == open_method::configured) {
            auto sm = g_config.get_open_method();
            if(sm == "silent") {
                method = open_method::silent;
            } else if(sm == "pick") {
                method = open_method::pick;
            } else {
                method = open_method::decide;
            }
        }

        if(method == open_method::pick) {
            picker(up, browser::to_instances(browser::get_cache()));
        } else {
            // silent or decide
            auto matches = browser::match(browser::get_cache(), up);
            browser_match_result& first_match = matches[0];
            up.app_mode = first_match.rule.app_mode;

            if(method == open_method::silent) {
                first_match.bi->launch(up);
                open_on_match_event(up, first_match);
            } else if(matches.size() == 1) {
                first_match.bi->launch(up);
                open_on_match_event(up, first_match);
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

        auto w = active_backend->make_window<config_window>();
        w->detach_on_close = true;
        w->on_open_changed = [](bool is_open) {
            is_config_running = is_open;
        };
        is_config_running = true;
    }

    void url_pipeline() {
        // allow only a single instance
        if(is_url_pipeline_running) return;

        prepare_ui_backend();

        auto w = active_backend->make_window<url_pipeline_window>();
        w->detach_on_close = true;
        w->on_open_changed = [](bool is_open) {
            is_url_pipeline_running = is_open;
        };
        is_url_pipeline_running = true;
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

        // SendMessaeg should return true if app processes the message, false otherwise
        return ::SendMessage(hwnd, WM_COPYDATA, (WPARAM)hwnd, (LPARAM)(LPVOID)&cds);
    }

    void contact() {
        url_open(
            url_payload{string(APP_URL) + "#contact"},
            ui::open_method::configured);
    }

    bool is_picker_hotkey_down() {
        string hk = g_config.get_picker_hotkey();

        if(hk == "cs") return win32::user::is_kbd_ctrl_down() && win32::user::is_kbd_shift_down();
        if(hk == "ca") return win32::user::is_kbd_ctrl_down() && win32::user::is_kbd_alt_down();
        if(hk == "as") return win32::user::is_kbd_alt_down() && win32::user::is_kbd_shift_down();

        return false;
    }
}