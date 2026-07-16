#include <format>
#include "globals.h"
#include "common/str.h"
#include "process.h"
#include "app/url_pipeline.h"
#include "app/rule_hit_log.h"
#include "app/url_opener.h"
#include "cmdline.h"
#include "app/discovery.h"

#if PLATFORM_WINDOWS
#include "win32/os.h"
#include "common/win32/window.h"
#endif

//ui
#include "app/ui/config_app.h"
#include "app/ui/picker_app.h"
#include "app/ui/toast_app.h"

using namespace std;
using namespace grey::common;
using namespace bt;

void open(bt::click_payload up, bool force_picker = false) {
    g_pipeline.process(up);

    // decision whether to show picker or not
    bool show_picker{force_picker};
    string pick_reason;
    vector<browser_match_result> rule_matches;
    if(!force_picker) {
        if(ui::picker_app::is_hotkey_down()) {
            show_picker = true;
            pick_reason = "hotkey";
        } else {
            rule_matches = browser::match(g_state.browsers, up, g_script);
            if(rule_matches.size() > 1) {
                show_picker = true;
                pick_reason = "conflict";
            } else if(rule_matches.size() == 1) {
                if(g_state.picker.invoke.on_no_rule && rule_matches[0].rule.is_fallback) {
                    show_picker = true;
                    pick_reason = "no rule";
                }
            }
        }
    }

    if(show_picker) {
        vector<profile_selection> limited_choices;
        if(!rule_matches.empty()) {
            for(auto& match : rule_matches) {
                limited_choices.push_back(match.profile);
            }
        }
        ui::picker_app app{up.url, limited_choices.empty() ? std::nullopt : std::make_optional(limited_choices)};
        auto pr = app.run();
        if(pr) {
            up.url = pr.url;
            url_opener::open(*pr.choice, up);
            if(g_state.log_rule_hits) {
                rule_hit_log::i.write(up, *pr.choice, "picker:" + pick_reason);
            }
        }
    } else {
        auto matches = bt::browser::match(g_state.browsers, up, g_script);
        browser_match_result& first_match = matches[0];
        first_match.rule.apply_to(up);
        url_opener::open(first_match.profile, up);
        if(g_state.log_rule_hits) {
            rule_hit_log::i.write(up, first_match.profile, matches[0].rule.to_string());
        }

        if(g_state.toast.enabled) {
            ui::toast_app app{up, first_match.profile};
            app.run();
        }
    }
}

string get_command(const string& data, string& command_data) {
    // Find the position of the first space character
    size_t pos = data.find(' ');

    // If no space is found, return the entire string
    if(pos == std::string::npos) {
        return "";
    }

    // Return the substring from the beginning up to the space character
    command_data = data.substr(pos + 1);
    return data.substr(0, pos);
}

void execute(const string& data) {

    // will be set to "true" if "pick" command is detected
    bool force_picker{false};
    string clean_data = data; // copy of the data, will be used for further processing


    if(data.empty() || data.starts_with(ArgSplitter)) {
        // if data starts with argsplitter that means command line is empty

        bt::ui::config_app app;
        app.run();

        return;
    }


    // try to extract command from the data, which is the first word in the data string
    string command_data;
    string command = get_command(data, command_data);
    if(!command.empty()) {
        if(command == "pick") {
            // force-invoke the picker
            force_picker = true;
            clean_data = command_data;
        } else if(command == "browser") {
            cmdline c;
            c.exec(command, command_data);
            return;
        } else if(command == "discover") {
            vector<bt::browser> fresh_browsers = bt::discovery::discover_all_browsers();
            fresh_browsers = bt::browser::merge(fresh_browsers, g_state.browsers);
            g_state.browsers = fresh_browsers;
            g_config.serialize();
            return;
        }
    }



    // if we reached this point, it's a normal operation mode
    // 0 - url
    // 1 - HWND
    auto parts = str::split(clean_data, ArgSplitter, true);
    bt::click_payload up{parts[0]};

    up.source_window_handle = (HWND)(DWORD)str::to_ulong(parts[1], 16);

#if PLATFORM_WINDOWS
    grey::common::win32::window win{up.source_window_handle};
    up.window_title = win.get_text();

    process proc{win.get_pid()};
    up.process_path = proc.get_module_filename();
    up.process_name = proc.get_name();
    up.process_description = proc.get_description();
#endif

#if _DEBUG
    if(command == "toast") {
        up.url = command_data;
        bt::ui::toast_app app{up, browser::get_default(g_state.browsers).value()};
        app.run();
        return;
    }
#endif

    open(up, force_picker);   // open-up hahaha
}

/**
 * @brief Parses incoming arguments and shapes them into universal command format.
 * @param argc 
 * @param argv 
 * @return 
 */

#if PLATFORM_WINDOWS
string parse_args(int argc, wchar_t* argv[]) {
    string arg;

    for(int i = 1; i < argc; i++) {
        string pt = str::to_str(wstring(argv[i]));
        if(!arg.empty()) arg += " ";
        arg += pt;
    }

    arg = format("{}{}{:x}",
      arg,
      ArgSplitter,
      (DWORD)(grey::common::win32::window::get_foreground().get_handle()));

    return arg;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
    string arg = parse_args(argc, argv);
    execute(arg);
    return 0;
}

#else

string parse_args(int argc, char* argv[]) {
    string arg;

    for(int i = 1; i < argc; i++) {
        string pt{argv[i]};
        if(!arg.empty()) arg += " ";
        arg += pt;
    }

    arg = format("{}{}{:x}",
      arg,
      ArgSplitter,
      123);

    return arg;
}

int main(int argc, char* argv[]) {
    string arg = parse_args(argc, argv);

    execute(arg);

    return 0;
}

#endif