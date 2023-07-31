#include "dash_window.h"
#include <fmt/core.h>

using namespace grey;

namespace bt {
    dash_window::dash_window(grey::grey_context& ctx) : grey::window{ctx, "Readiness Dashboard", 400, 160} {
        can_resize = false;
        recheck();
        center();
    }

    bool dash_window::recheck() {
        clear();
        checks = setup::get_checks();

        size_t succeeded{0}, failed{0};

        for(auto& sc : checks) {
            sc.recheck();

            auto g = make_group();
            auto sign = g->make_label(sc.is_ok ? ICON_FA_HEART_CIRCLE_CHECK : ICON_FA_HEART_CIRCLE_XMARK);
            sign->set_emphasis(sc.is_ok ? grey::emphasis::primary : grey::emphasis::error);

            g->same_line();
            g->make_label(sc.name);

            if(sc.is_ok) {
                succeeded += 1;
            } else {
                failed += 1;

                g->same_line();
                auto cmd_fix = g->make_button("fix", true);
                cmd_fix->on_pressed = [this, &sc](button&) {
                    sc.fix();
                    recheck();
                };
            }

            g->tooltip = sc.description;

            spacer();
        }

        if(failed == 0) {
            make_label("all checks succeeded");
        } else {
            make_label(fmt::format("{} checks failed out of {}", failed, succeeded + failed));
        }

        if(on_health_changed) {
            on_health_changed(failed == 0);
        }

        separator();
        auto cmd_recheck = make_button("recheck", false, grey::emphasis::primary);
        cmd_recheck->on_pressed = [this](button&) { recheck(); };

        same_line();
        auto cmd_close = make_button("close");
        cmd_close->on_pressed = [this](button&) {
            *is_visible = false;
        };

        return failed == 0;
    }
}