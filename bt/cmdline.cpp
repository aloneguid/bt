#include "cmdline.h"
#include <Windows.h>
#include <iostream>
#include "globals.h"
#include "str.h"

using namespace std;

cmdline::cmdline(){
    // Attach to the parent process's console or create a new one
    if(!::AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }

    // Redirect standard input, output, and error streams to the console
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}

int cmdline::exec(const std::string& command, const std::string& data) {
    // browser related queries
    if(data.starts_with("list|")) return exec_list();
    if(data.starts_with("get default|")) return exec_get_default();
    if(data.starts_with("set default ")) return exec_set_default(data);

    wcout << L"Unknown command: " << str::to_wstr(command) << endl;
    return 1;
}

int cmdline::exec_list() {
    // list all browsers and profiles

    wcout << L"browsers: " << g_config.browsers.size() << endl << endl;

    for(const auto& b : g_config.browsers) {
        wcout << str::to_wstr(b->id) << endl;
        wcout << L"  name:             " << str::to_wstr(b->name) << endl;
        wcout << L"  cmd:              " << str::to_wstr(b->open_cmd) << endl;
        wcout << L"  autodiscovered:   " << (b->is_autodiscovered ? L"yes" : L"no") << endl;
        wcout << L"  hidden:           " << (b->is_hidden ? L"yes" : L"no") << endl;
        if(b->is_autodiscovered || b->engine == bt::browser_engine::chromium || b->engine == bt::browser_engine::gecko) {
            wcout << L"  features: ";
            if(b->is_autodiscovered) {
                wcout << L"autodiscovered ";
            }
            if(b->engine == bt::browser_engine::chromium) {
                wcout << L"chromium ";
            }
            if(b->engine == bt::browser_engine::gecko) {
                wcout << L"gecko ";
            }
            wcout << endl;
        }
        if(!b->icon_path.empty()) {
            wcout << L"  icon:     " << str::to_wstr(b->icon_path) << endl;
        }

        wcout << L"  profiles: " << b->instances.size() << endl;
        for(const auto& p : b->instances) {
            wcout << L"    > id:        " << str::to_wstr(p->id) << endl;
            wcout << L"      name:      " << str::to_wstr(p->name) << endl;
            if(!p->launch_arg.empty()) {
                wcout << L"      args:      " << str::to_wstr(p->launch_arg) << endl;
            }
            if(!p->user_arg.empty()) {
                wcout << L"      uargs:     " << str::to_wstr(p->user_arg) << endl;
            }
            wcout << L"      hidden:    " << (p->is_hidden ? L"yes" : L"no") << endl;
            if(!p->icon_path.empty()) {
                wcout << L"      icon:      " << str::to_wstr(p->icon_path) << endl;
            }
            if(!p->user_icon_path.empty()) {
                wcout << L"      uicon:     " << str::to_wstr(p->user_icon_path) << endl;
            }
            wcout << L"      incognito: " << (p->is_incognito ? L"yes" : L"no") << endl;
        }

        wcout << endl;
    }

    return 0;
}

int cmdline::exec_get_default() {

    auto def = bt::browser::get_default(g_config.browsers, g_config.default_profile_long_id);

    wcout << str::to_wstr(def->b->id) << L"." << str::to_wstr(def->id) << endl;

    return 0;
}

int cmdline::exec_set_default(const std::string& data) {
    // data is in the following form: "set default <browser_id>.<profile_id>|suffix"
    // suffix is optional
    // extract browser_id and profile_id
    string browser_id;
    string profile_id;

    string t = data.substr(12); // skip "set default "
    size_t pos = t.find('|');
    if(pos != string::npos) {
        t = t.substr(0, pos);
    }
    size_t dot = t.find('.');
    if(dot != string::npos) {
        browser_id = t.substr(0, dot);
        profile_id = t.substr(dot + 1);
    }

    wcout << L"setting default browser to " << str::to_wstr(browser_id) << L"." << str::to_wstr(profile_id) << endl;

    // find browser
    for(const auto& b : g_config.browsers) {
        if(b->id == browser_id) {
            wcout << L"found browser: " << str::to_wstr(b->name) << endl;
            // find profile
            for(const auto& p : b->instances) {
                if(p->id == profile_id) {
                    wcout << L"found profile: " << str::to_wstr(p->name) << endl;
                    g_config.default_profile_long_id = p->long_id();
                    g_config.commit();
                    wcout << L"default profile set and saved." << endl;
                    return 0;
                }
            }
        }
    }

    wcout << L"browser or profile not found" << endl;
    return 1;
}
