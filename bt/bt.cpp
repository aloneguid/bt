#include <fmt/core.h>
#include "globals.h"
#include "../common/str.h"
#include "win32/process.h"
#include "app/config.h"
#include "app/url_pipeline.h"
#include "app/setup.h"
#include "win32/window.h"
#include "app/rule_hit_log.h"

//ui
#include "app/ui/configure.h"
#include "app/ui/picker.h"

// globals.h
//alg::tracker t{APP_SHORT_NAME, APP_VERSION};
bt::config g_config;
bt::url_pipeline g_pipeline{g_config};

using namespace std;

void execute(const string& data) {
    if(!data.empty() && !data.starts_with(ArgSplitter)) {
        // if data starts with argsplitter that means command line is empty

        // 0 - url
        // 1 - HWND
        auto parts = str::split(data, ArgSplitter, true);
        bt::url_payload up{parts[0]};

        //bool picker_down = bt::ui::is_picker_hotkey_down();
        //bt::ui::open_method om = picker_down ? bt::ui::open_method::pick : bt::ui::open_method::configured;

        up.source_window_handle = (HWND)(DWORD)str::to_ulong(parts[1], 16);

        win32::window win{up.source_window_handle};
        up.window_title = win.get_text();

        win32::process proc{win.get_pid()};
        up.process_name = proc.get_name();

        //bt::ui::url_open(up, om);
    } else {
        bt::ui::config_app app;
        //bt::ui::picker_app app{"https://github.com/sonnyp/Junction"};
        app.run();
    }
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