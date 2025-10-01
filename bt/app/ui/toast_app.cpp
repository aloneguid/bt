#include "toast_app.h"
#include "../../globals.h"
#include "../../res.inl"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    toast_app::toast_app(const std::string& text) :
        text{text},
        app{grey::app::make("toast", 100, 100)},
        wnd_main{"wtoast", &is_open} {
        app->initial_theme_id = g_config.theme_id;
        app->win32_can_resize = false;
        app->win32_always_on_top = true;
        app->win32_transparent = true;
        app->win32_hide_from_taskbar = true;

        wnd_main
            .no_titlebar()
            .no_resize()
            .border(0)
            .no_collapse()
            .fill_viewport()
            //.no_background()
            .no_scroll();

        app->on_initialised = [this]() {
            app->preload_texture("logo", icon_png, icon_png_len);
        };

    }

    void toast_app::size_to_fit() {
        if(stage == toast_app::anim_stage::init) {
            ImVec2 ts = ImGui::CalcTextSize(text.c_str());
            ImVec2 wpad = ImGui::GetStyle().WindowPadding;
            icon_size = ts.y;
            wnd_size = ImVec2{
                wpad.x * 2 + ts.x + icon_size * app->scale,
                wpad.y * 2 + ts.y};
            wnd_size_anim = ImVec2{0, wnd_size.y};  // only animate X

            int mon_idx = app->find_monitor_for_main_viewport();
            ImGuiPlatformIO io = ImGui::GetPlatformIO();
            ImGuiPlatformMonitor monitor = io.Monitors[mon_idx];
            mon_mid = ImVec2{
                monitor.WorkPos.x + (monitor.WorkSize.x / 2),
                monitor.WorkPos.y + monitor.WorkSize.y };

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
            if(show_timer >= ShowDuration) {
                stage = toast_app::anim_stage::shrink;
            }
        }
    }

    void toast_app::render_content() {
        w::image(*app, "logo", icon_size, icon_size);

        w::sl();
        w::label(text);
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