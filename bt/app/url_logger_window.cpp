#include "url_logger_window.h"
#include <windows.h>
#include "win32/window.h"
#include "../globals.h"
#include <iostream>

using namespace std;

namespace bt {
    url_logger_window::url_logger_window(grey::grey_context& ctx) : ctx{ctx}, grey::window{ctx, "URL Logger", 400, 200} {
        /*auto chk_aot = make_checkbox(ICON_FA_JET_FIGHTER_UP);
        chk_aot->render_as_icon = true;
        chk_aot->tooltip = "Always on top";
        chk_aot->on_value_changed = [this](bool on_top) {
            HWND hWnd = (HWND)this->ctx.get_native_window_handle();
            win32::window ww{hWnd};
            ww.set_topmost(on_top);
        };*/

        // make window to sctoll items
        w_events = make_child_window();

        open_on_match_event.connect(*this, this);   // will be disconnected by destructor as we are deriving from lsignal::slot
    }

    url_logger_window::~url_logger_window() {
    }

    void url_logger_window::operator()(const bt::url_payload& url, const bt::browser_match_result& bmr) {
        append(url, bmr);
    }

    void url_logger_window::append(const bt::url_payload& url, const bt::browser_match_result& bmr) {
        // link
        w_events->make_label(ICON_FA_LINK);
        w_events->same_line();
        w_events->make_label(url.url);

        // browser and profile
        w_events->make_label(ICON_FA_INTERNET_EXPLORER);
        w_events->same_line();
        w_events->make_label(bmr.bi->b->name);
        w_events->same_line();
        w_events->same_line();
        w_events->make_label("  ");
        w_events->make_label(ICON_FA_USER);
        w_events->same_line();
        w_events->make_label(bmr.bi->name);

        // rule
        w_events->make_label(ICON_FA_RULER);
        w_events->same_line();
        w_events->make_label(bmr.rule.value);

        // process
        if(!url.process_name.empty()) {
            w_events->make_label(ICON_FA_MICROCHIP);
            w_events->same_line();
            w_events->make_label(url.process_name);
        }

        // window title
        if(!url.window_title.empty()) {
            w_events->make_label(ICON_FA_WINDOW_MAXIMIZE);
            w_events->same_line();
            w_events->make_label(url.window_title);
        }

        w_events->separator();
    }
}