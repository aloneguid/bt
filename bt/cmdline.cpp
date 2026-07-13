#include "cmdline.h"
#include <iostream>
#include "globals.h"
#include "str.h"
#include "common/platform.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

using namespace std;
using namespace grey::common;

cmdline::cmdline(){
#if PLATFORM_WINDOWS
    // Attach to the parent process's console or create a new one
    if(!::AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }
#endif

    // Redirect standard input, output, and error streams to the console
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}

int cmdline::exec(const std::string& command, const std::string& data) {
    // browser-related queries
    if(data.starts_with("list|")) return exec_list();
    if(data.starts_with("get default|")) return exec_get_default();
    if(data.starts_with("set default ")) return exec_set_default(data);

    wcout << L"Unknown command: " << str::to_wstr(command) << endl;
    return 1;
}

int cmdline::exec_list() {
    // list all browsers and profiles

    wcout << L"browsers: " << g_state.browsers.size() << endl << endl;

    for(const auto& b : g_state.browsers) {
        string engine_name{magic_enum::enum_name(b.engine)};
        wcout << str::to_wstr(b.name) << endl;
        wcout << L"  cmd:              " << str::to_wstr(b.open_cmd) << endl;
        wcout << L"  engine:           " << str::to_wstr(engine_name) << endl;
        wcout << L"  hidden:           " << (b.is_hidden ? L"yes" : L"no") << endl;
        if(!b.icon_path.empty()) {
            wcout << L"  icon:     " << str::to_wstr(b.icon_path) << endl;
        }

        wcout << L"  profiles: " << b.profiles.size() << endl;
        for(const auto& p : b.profiles) {
            wcout << L"    > name:      " << str::to_wstr(p.name) << endl;
            if(!p.launch_arg.empty()) {
                wcout << L"      args:      " << str::to_wstr(p.launch_arg) << endl;
            }
            if(!p.user_arg.empty()) {
                wcout << L"      uargs:     " << str::to_wstr(p.user_arg) << endl;
            }
            wcout << L"      hidden:    " << (p.is_hidden ? L"yes" : L"no") << endl;
            if(!p.icon_path.empty()) {
                wcout << L"      icon:      " << str::to_wstr(p.icon_path) << endl;
            }
            if(!p.user_icon_path.empty()) {
                wcout << L"      uicon:     " << str::to_wstr(p.user_icon_path) << endl;
            }
            wcout << L"      incognito: " << (p.is_incognito ? L"yes" : L"no") << endl;
        }

        wcout << endl;
    }

    return 0;
}

int cmdline::exec_get_default() {

    optional<bt::profile_selection> ps = bt::browser::get_default(g_state.browsers);
    if(ps) {
        wcout << str::to_wstr(ps->b().name) << L"." << str::to_wstr(ps->profile().name) << endl;
    } else {
        wcout << L"no default browser" << endl;
    }
    return 0;
}

int cmdline::exec_set_default(const std::string& data) {
    // Data is in the following form: "set default <browser_name>.<profile_name>|suffix".
    // Suffix is optional.
    // Extract browser_id and profile_id.
    string browser_name;
    string profile_name;

    string t = data.substr(12); // skip "set default"
    size_t pos = t.find('|');
    if(pos != string::npos) {
        t = t.substr(0, pos);
    }
    size_t dot = t.find('.');
    if(dot != string::npos) {
        browser_name = t.substr(0, dot);
        profile_name = t.substr(dot + 1);
    }

    wcout << L"setting default browser to " << str::to_wstr(browser_name) << L"." << str::to_wstr(profile_name) << endl;

    // find browser
    for(const auto& b : g_state.browsers) {
        if(b.name == browser_name) {
            wcout << L"found browser: " << str::to_wstr(b.name) << endl;
            // find profile
            for(const auto& p : b.profiles) {
                if(p.name == profile_name) {
                    wcout << L"found profile: " << str::to_wstr(p.name) << endl;
                    g_state_container.serialize();
                    wcout << L"default profile set and saved." << endl;
                    return 0;
                }
            }
        }
    }

    wcout << L"browser or profile not found" << endl;
    return 1;
}
