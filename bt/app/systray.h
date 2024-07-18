#pragma once
#include <string>
#include "win32/popup_menu.h"

/**
 * @brief Application that runs in system tray notification area.
 */

namespace bt {
    class systray {
    public:
        void run();

    private:
        std::string last_clipboard_url;

        void build(win32::popup_menu& m);
        void handle_clipboard_text(const std::string& text);
    };
}