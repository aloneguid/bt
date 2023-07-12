#include "url_logger_window.h"
#include <windows.h>
#include "win32/window.h"
#include "../globals.h"
#include <iostream>

using namespace std;

namespace bt {
    url_logger_window::url_logger_window(grey::grey_context& ctx) : ctx{ctx}, grey::window{ctx, "URL Logger", 400, 200} {
        auto chk_aot = make_checkbox(ICON_FA_JET_FIGHTER_UP);
        chk_aot->render_as_icon = true;
        chk_aot->tooltip = "Always on top";
        chk_aot->on_value_changed = [this](bool on_top) {
            HWND hWnd = (HWND)this->ctx.get_native_window_handle();
            win32::window ww{hWnd};
            ww.set_topmost(on_top);
        };

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
        w_events->make_label(ICON_FA_LINK);
        w_events->same_line();
        w_events->make_label(url.url);
        
        w_events->make_label(bmr.bi->name);
        w_events->make_label(bmr.rule.value);
        w_events->make_label(url.process_name);
        w_events->make_label(url.window_title);


        w_events->separator();
    }
}