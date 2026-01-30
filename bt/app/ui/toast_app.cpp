#include "toast_app.h"
#include "../../globals.h"
#include "../../res.inl"
#include "btwidgets.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    toast_app::toast_app(const click_payload& cpp, std::shared_ptr<bt::browser_instance> bi) :
        cp{cpp}, bi{bi},
        app{grey::app::make("toast", 100, 100)},
        wnd_main{"wtoast", &is_open} {
        app->initial_theme_id = g_config.theme_id;
        app->win32_can_resize = false;
        app->win32_always_on_top = true;
        app->win32_transparent = true;
        app->win32_hide_from_taskbar = true;
        app->win32_no_activate = true;  // prevent from stealing focus or appearing in alt-tab

        wnd_main
            .no_titlebar()
            .no_resize()
            .border(g_config.toast_border_width)
            .no_collapse()
            .fill_viewport()
            //.no_background()
            .no_scroll();

        app->on_initialised = [this]() {
            app->preload_texture("logo", icon_png, icon_png_len);
            btw_on_app_initialised(*app);

            if(!cp.process_path.empty()) {
                app->preload_texture("app_icon", cp.process_path);
            }
        };

    }

    void toast_app::size_to_fit() {
        if(stage == toast_app::anim_stage::init) {

            // get monitor dimensions
            int mon_idx = app->find_monitor_for_main_viewport();
            if(mon_idx != -1) {
                ImGuiPlatformIO io = ImGui::GetPlatformIO();
                ImGuiPlatformMonitor monitor = io.Monitors[mon_idx];
                mon_work_pos = monitor.WorkPos;
                mon_work_size = monitor.WorkSize;
            }

            ImVec2 ts = ImGui::CalcTextSize(cp.url.c_str());
            ImVec2 wpad = ImGui::GetStyle().WindowPadding;
            icon_size = ts.y;
            float wnd_width = min(wpad.x * 2 + ts.x + icon_size * app->scale, mon_work_size.x - 20.0f);
            wnd_size = ImVec2{
                wnd_width,
                wpad.y * 2 + ts.y * 2};
            wnd_size_anim = ImVec2{0, wnd_size.y};  // only animate X

            mon_mid = ImVec2{
                mon_work_pos.x + (mon_work_size.x / 2),
                mon_work_pos.y + mon_work_size.y
            };

            stage = toast_app::anim_stage::expand;
        }

        // animate size

        if(stage == toast_app::anim_stage::expand) {
            wnd_size_anim.x += (wnd_size.x - wnd_size_anim.x) * AnimSpeed;
            if(fabs(wnd_size_anim.x - wnd_size.x) < 1.0f) wnd_size_anim.x = wnd_size.x;

            if(wnd_size_anim.x == wnd_size.x) {
                stage = toast_app::anim_stage::show;
            }

            app->resize_main_viewport((int)(wnd_size_anim.x / app->scale), (int)(wnd_size_anim.y / app->scale));
            app->move_main_viewport((mon_mid.x - wnd_size_anim.x / 2) / app->scale,
                (mon_mid.y - wnd_size_anim.y) / app->scale);
        } else if(stage == toast_app::anim_stage::shrink) {
            if(wnd_size_anim.x == wnd_size.x) {
                wnd_size_anim.x -= 1.0f; // start shrinking
            }

            wnd_size_anim.x -= (wnd_size.x - wnd_size_anim.x) * AnimSpeed;

            if(wnd_size_anim.x <= 1.0f) {
                stage = toast_app::anim_stage::exit;
                is_open = false;
            }

            app->resize_main_viewport((int)(wnd_size_anim.x / app->scale), (int)(wnd_size_anim.y / app->scale));
            app->move_main_viewport((mon_mid.x - wnd_size_anim.x / 2) / app->scale,
                (mon_mid.y - wnd_size_anim.y) / app->scale);
        } else if(stage == toast_app::anim_stage::show) {
            show_timer += ImGui::GetIO().DeltaTime;
            if(show_timer >= g_config.toast_visible_secs) {
                stage = toast_app::anim_stage::shrink;
            }
        }
    }

    void toast_app::render_content() {
        // line 1
        if(cp.process_path.empty()) {
            w::image(*app, "logo", icon_size, icon_size);
        } else {
            w::image(*app, "app_icon", icon_size, icon_size);
        }
        w::sl();
        if(!cp.process_description.empty()) {
            w::label(cp.process_description);
            if(!cp.process_name.empty()) {
                w::sl();
                w::label("(" + cp.process_name + ")");
            }
        } else if(!cp.process_name.empty()) {
            w::label(cp.process_name);
        } else {
            w::label("unknown", w::emphasis::error);
        }

        // line 2
        btw_icon(*app, bi, 0, icon_size, true);
        w::sl();
        w::label(cp.url);
    }

    void toast_app::run() {
        app->run([this](const grey::app& app1) {

            size_to_fit();

            {
                w::guard gw{wnd_main};

                render_content();

                //ImGui::ShowDemoWindow();

                // check if mouse cursor is over the window to pause the timer
                if(w::is_hovered()) {
                    show_timer = 0.0f;
                }
            }

            return is_open;
        });
    }

}