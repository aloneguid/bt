#include <fmt/core.h>
#include "globals.h"
#include "../common/str.h"
#include "../common/ext/alg_tracker.h"
#include "win32/process.h"
#include "app/config.h"
#include "app/url_pipeline.h"
#include "app/setup.h"
#include "win32/window.h"
#include "app/rule_hit_log.h"
#include "app/url_opener.h"

//ui
#include "app/ui/config_app.h"
#include "app/ui/picker_app.h"

// globals.h
alg::tracker t{APP_SHORT_NAME, APP_VERSION};
bt::config g_config;
bt::url_pipeline g_pipeline{g_config};

using namespace std;

void open(bt::url_payload up, bool force_picker = false) {

    bool picker_hotkeyed =
        g_config.picker_always || 
        bt::ui::picker_app::is_hotkey_down();

    g_pipeline.process(up);

    if(force_picker || picker_hotkeyed) {
        bt::ui::picker_app app{up.url};
        auto bi = app.run();
        if(bi) {
            bt::url_opener::open(bi, up);
            if(g_config.log_rule_hits) {
                bt::rule_hit_log::i.write(up, bi, "");
            }
        }
    } else {
        auto matches = bt::browser::match(g_config.browsers, up);
        bt::url_opener::open(matches[0].bi, up);
        if(g_config.log_rule_hits) {
            bt::rule_hit_log::i.write(up, matches[0].bi, matches[0].rule.to_line());
        }
    }

    track_event("click");
}

void track_event(string name) {
    t.track(map<string, string> {
        {"event", name}
    }, true);
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
        track_event("config");

        return;
    }


    // try to extract command from the data, which is the first word in the data string
    string command_data;
    string command = get_command(data, command_data);
    if(!command.empty()) {
        // force-invoke the picker
        if(command == "pick") {
            force_picker = true;
            clean_data = command_data;
        }
    }



    // if we reached this point, it's a normal operation mode
    // 0 - url
    // 1 - HWND
    auto parts = str::split(clean_data, ArgSplitter, true);
    bt::url_payload up{parts[0]};

    up.source_window_handle = (HWND)(DWORD)str::to_ulong(parts[1], 16);

    win32::window win{up.source_window_handle};
    up.window_title = win.get_text();

    win32::process proc{win.get_pid()};
    up.process_name = proc.get_name();

    open(up, force_picker);   // open-up hahaha
}

/**
 * @brief Shows a message box with the command line arguments (for debugging purposes)
 * @param argc 
 * @param argv 
 */
void debug_args_msgbox(int argc, wchar_t* argv[]) {
    if(g_config.get_flag("debug_args") == "y") {
        wostringstream msg;
        msg << "count: " << argc << endl;
        for(int i = 0; i < argc; i++) {
            msg << i + 1 << ": [" << argv[i] << "]" << endl;
        }
        auto fg = win32::window::get_foreground();
        win32::process p{fg.get_pid()};
        msg << "by: [" << str::to_wstr(p.get_name()) << "]";

        ::MessageBox(nullptr, msg.str().c_str(), L"Command Line Debugger", MB_OK);
    }
}

/**
 * @brief Parses incoming arguments and shapes them into universal command format.
 * @param argc 
 * @param argv 
 * @return 
 */
string parse_args(int argc, wchar_t* argv[]) {
    string arg;

    for(int i = 1; i < argc; i++) {
        string pt = str::to_str(wstring(argv[i]));
        if(!arg.empty()) arg += " ";
        arg += pt;
    }

    arg = fmt::format("{}{}{:x}",
      arg,
      ArgSplitter,
      (DWORD)(win32::window::get_foreground().get_handle()));

    return arg;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    debug_args_msgbox(argc, argv);

    string arg = parse_args(argc, argv);

    execute(arg);

    return 0;
}