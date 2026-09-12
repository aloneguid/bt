#include "toast_app.h"
#include "../../globals.h"
#include "../../res.h"
#include "btwidgets.h"
#include "platform.h"
#include "common/str.h"
#include <cmath>

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
        app->chrome = system_chrome::native;
#if PLATFORM_WINDOWS
        app->win32_hide_from_taskbar = true;
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
            int mon_idx = app->find_monitor_for_main_viewport();
            if(mon_idx != -1) {
                ImGuiPlatformIO io = ImGui::GetPlatformIO();
                ImGuiPlatformMonitor monitor = io.Monitors[mon_idx];
                mon_work_pos = monitor.WorkPos;
                mon_work_size = monitor.WorkSize;
            }

            ImGuiStyle& style = ImGui::GetStyle();
            auto space = style.FramePadding;

            // What toast is going to display:

            // Line 1: Caller icon + caller info
            string line1 = cp.process_description;
            if(line1.empty()) line1 = cp.process_name;
            if(line1.empty()) line1 = "Unknown";
            sz line1_text_size = w::text_size_get(line1);
            icon_size = line1_text_size.height;

            // Line 2: Profile icon + url domain or path + optional rule icon
            sz line2_text_size = w::text_size_get(cp_url_parsed.host.empty() ? cp_url_parsed.path : cp_url_parsed.host);
            if(!bmr.rule.is_fallback)
                line2_text_size.width += space.x + line2_text_size.height + space.x;

            ImVec2 wpad = style.WindowPadding;
            float wnd_width =
                min(wpad.x + icon_size + space.x + max(line1_text_size.width, line2_text_size.width) + wpad.x,
                        mon_work_size.x - 20.0f) +
                // add extra spacing
                wpad.x;

            // 2 lines of text + padding
            wnd_size = ImVec2{
                wnd_width,
                wpad.y + line1_text_size.height + space.y + line2_text_size.height + wpad.y
            };
            wnd_size_anim = ImVec2{0, wnd_size.y}; // only animate X

            mon_mid = ImVec2{
                mon_work_pos.x + (mon_work_size.x / 2),
                mon_work_pos.y + mon_work_size.y
            };

            stage = anim_stage::expand;
        }

        // animate size

        if(stage == anim_stage::expand) {
            float move = (wnd_size.x / g_state.toast.anim_duration) * ImGui::GetIO().DeltaTime;
            wnd_size_anim.x += move;
            if(wnd_size_anim.x >= wnd_size.x) wnd_size_anim.x = wnd_size.x;

            if(wnd_size_anim.x == wnd_size.x) {
                stage = anim_stage::show;
            }

            app->resize({wnd_size_anim.x / w::scale, wnd_size_anim.y / w::scale});
            app->move({(mon_mid.x - wnd_size_anim.x / 2) / w::scale,
                                    (mon_mid.y - wnd_size_anim.y) / w::scale});
        } else if(stage == anim_stage::shrink) {
            float move = (wnd_size.x / g_state.toast.anim_duration) * ImGui::GetIO().DeltaTime;
            wnd_size_anim.x -= move;
            // don't let size to be 0
            if(wnd_size_anim.x <= 2.0f) wnd_size_anim.x = 2.0f;

            if(wnd_size_anim.x <= 2.0f) {
                wnd_size_anim.x = 2.0f;
                stage = anim_stage::exit;
                is_open = false;
            } else {
                app->resize({wnd_size_anim.x / w::scale, wnd_size_anim.y / w::scale});
                app->move({(mon_mid.x - wnd_size_anim.x / 2) / w::scale,
                                        (mon_mid.y - wnd_size_anim.y) / w::scale});
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
    }

    void toast_app::run() {
        app->run([this]() {
            size_to_fit();

            app->transparency_window_alpha = is_hovered ? 255 : g_state.toast.opacity;

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

            return is_open;
        });
    }
}
