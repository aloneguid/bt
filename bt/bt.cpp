#include "bt.h"
#include <fmt/core.h>
#include "globals.h"
#include "app/ui.h"
#include "../common/win32/npipe.h"
#include "../common/win32/user.h"
#include "../common/win32/kernel.h"
#include "../common/fss.h"
#include "../common/str.h"
#include "win32/app.h"
#include "win32/process.h"
#include "win32/shell_notify_icon.h"
#include "win32/popup_menu.h"
#include "app/config.h"
#include "app/setup.h"
#include "app/update_check.h"
#include "win32/window.h"
#include "app/rule_hit_log.h"

#define OWN_WM_NOTIFY_ICON_MESSAGE WM_APP + 1

// {365F3F68-6330-4D4F-BEF2-999EF15F1BE4}
static const GUID NotifyIconGuid = {0x365f3f68, 0x6330, 0x4d4f, { 0xbe, 0xf2, 0x99, 0x9e, 0xf1, 0x5f, 0x1b, 0xe4 }};

// globals.h
alg::tracker t{APP_SHORT_NAME, APP_VERSION};
lsignal::signal<void(const std::string&, const std::string&, const std::string&)> app_event;
lsignal::signal<void(const bt::url_payload&, const bt::browser_match_result&)> open_on_match_event;

using namespace std;

void execute(const string& data) {
    if (data.starts_with("shutdown")) {
        ::PostQuitMessage(0);
    } else if(data.starts_with("purge")) {
        bt::setup::unregister_all();
        ::PostQuitMessage(0);
    } else if(!data.empty() && !data.starts_with(ArgSplitter)) {
        // if data starts with argsplitter that means command line is empty

        // 0 - url
        // 1 - HWND
        auto parts = str::split(data, ArgSplitter, true);
        bt::url_payload up{parts[0]};
        bool picker_down = bt::ui::is_picker_hotkey_down();
        bt::ui::open_method om = picker_down ? bt::ui::open_method::pick : bt::ui::open_method::configured;
        up.source_window_handle = (HWND)(DWORD)str::to_ulong(parts[1], 16);

        win32::window win{up.source_window_handle};
        up.window_title = win.get_text();

        win32::process proc{win.get_pid()};
        up.process_name = proc.get_name();

        bt::ui::url_open(up, om);
    } else {
        bt::ui::config();
    }
}

void CALLBACK KeepAliveTimerProc(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime) {
    t.track(map<string, string> {
        { "event", "ping" }
    }, true);
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {

    if(bt::config::i.get_flag("debug_args") == "y") {
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

    if(bt::ui::try_invoke_running_instance(arg)) {
        return 0;
    }

    // if we have reached this point, it's a primary instance
    bt::ui::set_main_instance();

    win32::app win32app{Win32ClassName, AppGuid};

    {
        // make us energy efficient
        win32::process p;
        p.set_priority(IDLE_PRIORITY_CLASS);
        p.enable_efficiency_mode();
    }

    win32::shell_notify_icon sni{
        win32app.get_hwnd(),
            NotifyIconGuid,
            OWN_WM_NOTIFY_ICON_MESSAGE,
            fmt::format("{} {}", APP_LONG_NAME, APP_VERSION)};

    win32::popup_menu m{win32app.get_hwnd()};
    m.add("cfg", "Configure");
    m.add("$", "Buy me a coffee!");
    m.add("contact", "Contact");
    m.add("?", "Project website");
    m.add("doc", "Documentation");
    m.separator();
    m.add("x", "&Exit");

    win32app.on_app_window_message = [&m, &win32app](UINT msg, WPARAM wParam, LPARAM lParam) {
        switch(msg) {
            case OWN_WM_NOTIFY_ICON_MESSAGE:
                switch(lParam) {
                    case WM_LBUTTONUP:
                        bt::ui::config();
                        break;
                    case WM_RBUTTONUP:
                        // show context menu
                        m.show();
                        break;
                    case NIN_BALLOONUSERCLICK:
                        bt::ui::url_open(bt::url_payload{string(APP_URL) + "#installing"}, bt::ui::open_method::configured);
                        break;
                }
                break;
            case WM_COMMAND: {
                int loword_wparam = LOWORD(wParam);
                string id = m.id_from_loword_wparam(loword_wparam);
                if(id == "cfg") {
                    bt::ui::config();
                } else if(id == "x") {
                    ::PostQuitMessage(0);
                } else if(id == "$") {
                    bt::ui::coffee("shell_icon");
                } else if(id == "contact") {
                    bt::ui::contact();
                } else if(id == "?") {
                    bt::ui::url_open(bt::url_payload{APP_URL}, bt::ui::open_method::configured);
                } else if(id == "doc") {
                    bt::ui::url_open(bt::url_payload{APP_DOCS_URL}, bt::ui::open_method::configured);
                }
                }
                break;
            case WM_COPYDATA:
                COPYDATASTRUCT* cds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
                string command(static_cast<const char*>(cds->lpData));
                execute(command);
                // https://docs.microsoft.com/en-us/windows/win32/dataxchg/using-data-copy
                return 1;   // processed!

        }
        return 0;
    };

    win32app.on_message_loop_message = [](MSG& msg) {
        bt::ui::render_ui_frame_if_required();
        return 0;
    };

    bt::ui::on_ui_open_changed = [&win32app](bool is_open) {
        //win32app.set_max_fps_mode(is_open);
        win32app.set_message_timeout(is_open ? 100 : -1);
    };

    //bt::ui::launch_or_pick(bt::url_payload{"https://aloneguid.uk"}, bt::ui::open_method::pick);

    execute(arg);

    // set 1h
    ::SetTimer(win32app.get_hwnd(), 1,
        1000 * 60 * 60, // ms * sec * min
        KeepAliveTimerProc);

    app_event.connect([&sni](const string& name, const string& arg1, const string& arg2) {
        if(name == "new_version") {
            sni.display_notification("New Version", fmt::format("Version {} is now available!", arg1));
        } else if(name == "no_new_version") {
            sni.display_notification("Up to Date", "No new version available, you are up to date!");
        }
    });

    open_on_match_event.connect([&sni](const bt::url_payload& url, const bt::browser_match_result& bmr) {
        if(bt::config::i.get_log_rule_hits()) {
            bt::rule_hit_log::i.write(url, bmr);
        }
    });

    // check for new versions
    string vn;
    if(bt::app::should_check_new_version() && bt::app::has_new_version(vn)) {
        app_event("new_version", vn, "");
    }

    win32app.run();

    return 0;
}