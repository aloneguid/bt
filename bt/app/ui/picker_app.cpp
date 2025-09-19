#include "picker_app.h"
#include <memory>
#include "fss.h"
#include "../../globals.h"
#include "../../res.inl"
#include "fmt/core.h"
#include "../common/win32/user.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "../../common/win32/clipboard.h"
#include "../../common/win32/shell.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    picker_app::picker_app(const string& url) 
        : url{url}, title{APP_LONG_NAME " - Pick"},
        app{grey::app::make(title, WindowSize, WindowSize)},
        wnd_main{title, &is_open},
        wnd_settings{"Settings"} {
        app->initial_theme_id = g_config.theme_id;
        app->win32_can_resize = false;
        app->win32_center_on_screen = true;
        app->win32_close_on_focus_lost = g_config.picker_close_on_focus_loss;
        app->win32_always_on_top = g_config.picker_always_on_top;
        //app->win32_transparent = true;
        auto cc = app->get_clear_color();
        ImU32 cc1 = w::rgb_colour{ImVec4(cc[0], cc[1], cc[2], cc[3])};
        clear_color = cc1;

        // process URL with pipeline
        {
            click_payload up{url};
            g_pipeline.process(up);
            this->url = up.url;
        }

        choices = browser::to_instances(g_config.browsers, true);

        app->on_initialised = [this]() {

            app->preload_texture("incognito", incognito_icon_png, incognito_icon_png_len);
            app->preload_texture("more", picker_more_icon_png, picker_more_icon_png_len);

            // preload browser icons
            for(auto& b : g_config.browsers) {
                string path = b->get_best_icon_path();
                app->preload_texture(path, fss::get_full_path(path));

                // preload browser instances
                for(auto& bi : b->instances) {
                    string path = bi->get_best_icon_path();
                    app->preload_texture(path, fss::get_full_path(path));
                }
            }

            //float window_size_unscaled = outer_radius * 4.0f / app->scale;  // unscaled double diameter
            //app->resize_main_viewport(window_size_unscaled, window_size_unscaled);

            wnd_main
                //.size(wnd_width, wnd_height_normal)
                .no_titlebar()
                .no_resize()
                .border(0)
                .fill_viewport()
                .no_background()
                .no_scroll();

            wnd_settings
                .no_collapse()
                .size(WindowSize, 0)
                .no_scroll()
                .border(1);
        };

    }

    picker_app::~picker_app() {
    }

    picker_result picker_app::run() {
        app->run([this](const grey::app& app) {
            return run_frame();
        });

        return picker_result{decision, url};
    }

    bool picker_app::is_hotkey_down() {
        bool k_shift = win32::user::is_kbd_shift_down();
        bool k_ctrl = win32::user::is_kbd_ctrl_down();
        bool k_alt = win32::user::is_kbd_alt_down();
        bool k_caps = win32::user::is_kbd_caps_locks_on();

        return
            (g_config.picker_on_key_as && (k_alt && k_shift)) ||
            (g_config.picker_on_key_ca && (k_ctrl && k_alt)) ||
            (g_config.picker_on_key_cs && (k_ctrl && k_shift)) ||
            (g_config.picker_on_key_cl && k_caps);
    }

    bool picker_app::run_frame() {

        // inspiration: https://github.com/sonnyp/Junction

        w::guard gw{wnd_main};

        w::input(url, "##url", true, -FLT_MIN);
        render_action_menu();
        with_container(cnt_blist,
            render_list();
        );

        if(settings_open) {
            render_settings();
        }

        return is_open;
    }

    void picker_app::render_action_menu() {

        w::icon_checkbox(ICON_MD_SETTINGS, settings_open);

        for(const action_menu_item& ami : action_menu_items) {
            w::sl();
            if(w::button(ami.icon)) {
                menu_item_clicked(ami.id);
            }
            w::tooltip(ami.tooltip);
        }
    }

    void picker_app::render_list() {

        float padding = g_config.picker_item_padding * app->scale;
        float icon_size = g_config.picker_icon_size * app->scale;

        for(auto& p : choices) {
            {
                w::group g;
                g
                    .border_hover(ImGuiCol_HeaderActive)
                    .spread_horizontally()
                    .render();

                // render icon and come back to starting position
                float x0, y0;
                w::cur_get(x0, y0);

                // dummy
                ImGui::Dummy(ImVec2{1, icon_size + padding * 2});

                w::cur_set(x0 + padding, y0 + padding);
                w::rounded_image(*app, p->b->get_best_icon_path(), icon_size, icon_size, icon_size / 2);

                // if required, draw profile icon
                if(!p->is_singular()) {
                    w::cur_set(x0 + padding + icon_size / 2, y0 + padding + icon_size / 2);
                    float isz = icon_size / 2;
                    if(p->is_incognito) {
                        w::image(*app, "incognito", isz, isz);
                    } else {
                        w::rounded_image(*app, p->get_best_icon_path(), isz, isz, isz);
                    }
                }

                // draw label
                w::cur_set(x0 + icon_size + padding * 3, y0 + padding + icon_size / 2 - ImGui::GetTextLineHeight() / 2);
                w::label(p->get_best_display_name());
            }
        }
    }

    void picker_app::render_settings() {

        ImVec2 me_pos = ImGui::GetWindowPos();
        ImVec2 me_size = ImGui::GetWindowSize();

        ImGui::SetNextWindowPos(ImVec2{me_pos.x + me_size.x + 10 * app->scale, me_pos.y});
        {
            w::guard gw{wnd_settings};
            w::slider(g_config.picker_icon_size, 5, 256, "icon size");
            w::slider(g_config.picker_item_padding, 0, 100, "padding");

            if(w::button("reset")) {
                g_config.picker_icon_size = 32;
                g_config.picker_item_padding = 10;
            }
        }

    }

    void picker_app::menu_item_clicked(const std::string& id) {
        if(id == "copy") {
            win32::clipboard::set_ascii_text(url);
            is_open = false;
        } else if(id == "email") {
            win32::clipboard::set_ascii_text(url);
            win32::shell::exec(fmt::format("mailto:?body={}", url), "");
            is_open = false;
        } else if(id == "edit") {

        }
    }
}