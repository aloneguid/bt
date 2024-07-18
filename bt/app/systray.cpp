#include "systray.h"
#include "win32/app.h"
#include "win32/shell_notify_icon.h"
#include <fmt/core.h>

namespace bt {

    using namespace std;

    const string Win32ClassName("BTTrayApp");
    const string AppGuid("026741D2-FF77-462B-AD70-4140697C8AE1");
    #define OWN_WM_NOTIFY_ICON_MESSAGE WM_APP + 1
    // {365F3F68-6330-4D4F-BEF2-999EF15F1BE4}
    static const GUID NotifyIconGuid = {0x365f3f68, 0x6330, 0x4d4f, { 0xbe, 0xf2, 0x99, 0x9e, 0xf1, 0x5f, 0x1b, 0xe4 }};

    void systray::run() {
        win32::app app{Win32ClassName, AppGuid};
        win32::shell_notify_icon sni{
            app.get_hwnd(),
            NotifyIconGuid,
            OWN_WM_NOTIFY_ICON_MESSAGE,
            fmt::format("{} {}", APP_LONG_NAME, APP_VERSION)};
        win32::popup_menu m{app.get_hwnd()};

        app.on_app_window_message = [this, &m, &app](UINT msg, WPARAM wParam, LPARAM lParam) {
            switch(msg) {
                case OWN_WM_NOTIFY_ICON_MESSAGE:
                    switch(lParam) {
                        case WM_LBUTTONUP:
                        case WM_RBUTTONUP:
                            // show context menu
                            build(m);
                            m.show();
                            break;
                    }
                    break;
                case WM_COMMAND: {
                    int loword_wparam = LOWORD(wParam);
                    string id = m.id_from_loword_wparam(loword_wparam);
                    if(id == "x") {
                        ::PostQuitMessage(0);
                    } }
                    break;
                case WM_CLIPBOARDUPDATE:
                    // get string placed into the clipboard
                    if (::OpenClipboard(app.get_hwnd())) {
                        HANDLE hData = ::GetClipboardData(CF_TEXT);
                        if (hData) {
                            char* pszText = static_cast<char*>(::GlobalLock(hData));
                            if (pszText) {
                                handle_clipboard_text(pszText);
                                ::GlobalUnlock(hData);
                            }
                        }
                        ::CloseClipboard();
                    }
                    break;

            }
            return 0;
        };

        app.add_clipboard_listener();

        app.run();
    }

    void systray::build(win32::popup_menu& m) {
        // this menu can be rebuilt on the fly when needed (clear, add items)
        m.clear();
        m.add("o", last_clipboard_url);
        m.separator();
        m.add("x", "&Exit");
    }

    void systray::handle_clipboard_text(const std::string& text) {
        // todo: check if this is a URL indeed
        last_clipboard_url = text;
    }
}