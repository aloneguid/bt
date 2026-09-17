#include "toast_app.h"
#include "../../globals.h"
#include "../../res.h"
#include "btwidgets.h"
#include "platform.h"
#include "common/str.h"
#include <cmath>

#include "clipboard.h"

using namespace std;
namespace w = grey::widgets;
using namespace grey;
using namespace grey::common;

namespace bt::ui {
    toast_app::toast_app(const click_payload& cpp, const browser_match_result& bmr) : cp{cpp}, cp_url_parsed{cpp.url},
        bmr{bmr},
        app{app::make("toast", sz{100, 100})} {
        app->fonts.load_all();
        app->initial_theme_id = g_state.ui_theme;
        app->can_resize = false;

        app->always_on_top = true;
        app->chrome = system_chrome::headerless;
        app->hide_from_taskbar = true;

#if PLATFORM_WINDOWS
        app->win32_no_activate = true; // prevent from stealing focus or appearing in alt-tab
#endif

        auto& opts = app->main_window_opts();
        opts.open_ptr = &is_open;
        opts.border = g_state.toast.border_width;
        opts.scrollable = false;

        app->on_initialised = [this]() {
            app->preload_texture("logo", icon_png, icon_png_len);
            btw_on_app_initialised(*app);

            if(!cp.process_path.empty()) {
                app->preload_texture("app_icon", cp.process_path);
            }
        };
    }

    void toast_app::size_to_fit() {
        if(stage == anim_stage::init) {
            // Init phase happens only once.
            // Get perfect dimensions for expanded toast.

            // get monitor dimensions
            const monitor mon = w::mon_wnd();

            ImGuiStyle& style = ImGui::GetStyle();
            auto space = style.FramePadding;

            // What toast is going to display:

            // Line 1: Caller icon + caller info
            string line1 = cp.process_description;
            if(line1.empty()) line1 = cp.process_name;
            if(line1.empty()) line1 = "Unknown";
            sz line1_text_size = w::text_size_get(line1);
            icon_size = line1_text_size.height;

            // Line 2: Profile icon + url domain or path + optional rule icon + optional "tracker removed" icon
            sz line2_text_size = w::text_size_get(cp_url_parsed.host.empty() ? cp_url_parsed.path : cp_url_parsed.host);
            if(!bmr.rule.is_fallback)
                line2_text_size.width += space.x + line2_text_size.height + space.x;
            if(cp.trackers_removed > 0)
                line2_text_size.width += space.x + line2_text_size.height + space.x;

            ImVec2 wpad = style.WindowPadding;
            float wnd_width =
                min(wpad.x + icon_size + space.x + max(line1_text_size.width, line2_text_size.width) + wpad.x,
                        mon.work_area.width() - 20.0f) +
                // add extra spacing
                wpad.x;

            // 2 lines of text + padding
            wnd_size = {wnd_width,
                wpad.y + line1_text_size.height + space.y + line2_text_size.height + wpad.y
            };
            wnd_size_anim = {0, wnd_size.height}; // only animate X

            mon_mid = {
                mon.work_area.x_min + (mon.work_area.width() / 2),
                mon.work_area.y_min + mon.work_area.height()
            };

            stage = anim_stage::expand;
        }

        // animate size

        if(stage == anim_stage::expand) {
            float move = (wnd_size.width / g_state.toast.anim_duration) * ImGui::GetIO().DeltaTime;
            wnd_size_anim.width += move;
            if(wnd_size_anim.width >= wnd_size.width) wnd_size_anim.width = wnd_size.width;

            if(wnd_size_anim.width == wnd_size.width) {
                stage = anim_stage::show;
            }

            app->resize(wnd_size_anim);
            app->move({(mon_mid.width - wnd_size_anim.width / 2),
                                    (mon_mid.height - wnd_size_anim.height)});
        } else if(stage == anim_stage::shrink) {
            float move = (wnd_size.width / g_state.toast.anim_duration) * ImGui::GetIO().DeltaTime;
            wnd_size_anim.width -= move;
            // don't let size to be 0
            if(wnd_size_anim.width <= 2.0f) wnd_size_anim.width = 2.0f;

            if(wnd_size_anim.width <= 2.0f) {
                wnd_size_anim.width = 2.0f;
                stage = anim_stage::exit;
                is_open = false;
            } else {
                app->resize(wnd_size_anim);
                app->move({(mon_mid.width - wnd_size_anim.width / 2),
                                        (mon_mid.height - wnd_size_anim.height)});
            }
        } else if(stage == anim_stage::show) {
            show_timer += ImGui::GetIO().DeltaTime;
            if(show_timer >= g_state.toast.visible_seconds) {
                stage = anim_stage::shrink;
            }
        }
    }

    void toast_app::render_content() const {
        // --- line 1

        {
            w::group g_line1;

            // small icon
            if(cp.process_path.empty()) {
                w::image(*app, "logo", sz::square(icon_size));
            } else {
                w::image(*app, "app_icon", sz::square(icon_size));
            }

            w::sl();

            // process description, name, or "unknown"
            if(!cp.process_description.empty()) {
                w::lbl(cp.process_description, {.emp = emphasis::primary});
            } else if(!cp.process_name.empty()) {
                w::lbl(cp.process_name, {.emp = emphasis::primary});
            } else {
                w::lbl("unknown", {.emp = emphasis::error});
            }
        }
        if(w::is_hovered()) {
            w::rich_tt rtt;

            float col1_start = 80 * w::scale;

            if(!cp.process_id.empty()) {
                w::lbl("id:");
                w::sl(col1_start);
                w::lbl(cp.process_id, {.emp = emphasis::primary});
            }

            if(!cp.process_name.empty()) {
                w::lbl("name:");
                w::sl(col1_start);
                w::lbl(cp.process_name, {.emp = emphasis::primary});
            }

            if(!cp.process_description.empty()) {
                w::lbl("description:");
                w::sl(col1_start);
                w::lbl(cp.process_description, {.emp = emphasis::primary});
            }

            if(!cp.process_path.empty()) {
                w::lbl("path:");
                w::sl(col1_start);
                w::lbl(cp.process_path, {.emp = emphasis::primary});
            }

            if(!cp.window_title.empty()) {
                w::lbl("title:");
                w::sl(col1_start);
                w::lbl(cp.window_title, {.emp = emphasis::primary});
            }
        }

        // --- line 2

        // profile icon
        {
            w::group g_icon;
            btw_icon(*app, bmr.profile, 0, 0, icon_size);
        }
        if(w::is_hovered()) {
            w::rich_tt rtt;

            w::lbl("browser: ");
            w::sl(80 * w::scale);
            w::lbl(bmr.profile.b().name, {.emp = emphasis::primary});

            w::lbl("profile: ");
            w::sl(80 * w::scale);
            w::lbl(bmr.profile.p().name, {.emp = emphasis::primary});
        }

        w::sl();
        w::lbl("");

        // short version of URL
        if(!cp_url_parsed.scheme.empty()) {
            w::sl(0, false);
            w::texter tx{0, font_weight::bold};
            if(!cp_url_parsed.host.empty()) {
                w::lbl(cp_url_parsed.host);
            } else {
                w::lbl(cp_url_parsed.path);
            }
        }

        if(w::is_hovered()) {
            w::rich_tt rtt;

            const auto& url = cp_url_parsed;
            w::lbl(url.to_string());
        }

        if(!bmr.rule.is_fallback) {
            w::sl();
            w::lbl(ICON_MD_RULE, {.emp = emphasis::primary});
            w::tt(bmr.rule.to_string());
        }

        if(cp.trackers_removed > 0) {
            w::sl();
            w::spinner(spinner_type::solar_scale_balls, {
                .emp = emphasis::error,
                .radius = w::scaled(8),
                .speed = 2.0f});
            if(w::is_hovered()) {
                w::mouse_cursor(w::mouse_cursor_type::hand);
                if(w::is_leftclicked()) {
                    clipboard::set_text(format("{}\n{}", cp.raw_url, cp.url));
                    w::toast(emphasis::info, "URLs copied to clipboard.");
                }

                w::rich_tt rtt;

                w::lbl(cp.raw_url, {.emp = emphasis::secondary, .center_x = true});
                w::lbl(ICON_MD_ARROW_DOWNWARD, {.emp = emphasis::info, .center_x = true});
                w::lbl(cp.url, {.emp = emphasis::primary, .center_x = true});
                w::spc();
                w::sep();
                w::lbl("Click to copy both URLs to clipboard.", {.emp = emphasis::disabled, .center_x = true});
            }
        }
    }

    void toast_app::run() {
        app->run([this]() {
            size_to_fit();

            app->opacity = is_hovered ? 1.0f : g_state.toast.opacity;

            {
                {
                    w::group g;
                    render_content();
                }

                is_hovered = w::is_hovered();

                // check if mouse cursor is over the window to pause the timer
                if(is_hovered) {
                    show_timer = 0.0f;
                }
            }

            w::toast_render_frame();

            return is_open;
        });
    }
}
