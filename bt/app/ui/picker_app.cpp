#include "picker_app.h"
#include <memory>
#include "fss.h"
#include "../../globals.h"
#include "../../res.inl"
#include "fmt/core.h"
#include "../common/win32/user.h"
#include "../strings.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "../../common/win32/clipboard.h"
#include "../../common/win32/shell.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    picker_app::picker_app(const string& url) 
        : url{url}, title{APP_LONG_NAME " - Pick"},
        app{grey::app::make(title, WindowSize, WindowSize)}, wnd_main{ title, &is_open } {
        app->initial_theme_id = g_config.theme_id;
        //app->load_icon_font = false;
        app->win32_can_resize = false;
        app->win32_center_on_screen = true;
        app->win32_close_on_focus_lost = g_config.picker_close_on_focus_loss;
        app->win32_always_on_top = g_config.picker_always_on_top;
        app->win32_transparent = true;

        // process URL with pipeline
        {
            click_payload up{url};
            g_pipeline.process(up);
            this->url = up.url;
        }

        choices = browser::to_instances(g_config.browsers, true);
        menu_radius = get_circle_radius_for_icons(
            static_cast<int>(choices.size()),
            (IconRadius + IconPadding) * app->scale);
        inner_radius = menu_radius - (IconRadius * 2 * app->scale);
        outer_radius = menu_radius + (IconRadius * 2 * app->scale);
        action_menu_radius = get_circle_radius_for_icons(
            static_cast<int>(action_menu_items.size()),
            (IconRadius + IconPadding) * app->scale);
        
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

            wnd_main
                //.size(wnd_width, wnd_height_normal)
                .no_titlebar()
                .no_resize()
                .border(0)
                .fill_viewport()
                .no_background()
                .no_scroll();
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

    float picker_app::get_circle_radius_for_icons(int icon_count, float icon_radius) {
        if(icon_count <= 0) return 0.0f;
        if(icon_count == 1) return 0.0f; // Single icon at center
        if(icon_count == 2) return icon_radius; // Two icons side by side

        // For N > 2: R_circle = R_icon / sin(π/N)
        float angle_per_icon = M_PI / icon_count;
        return icon_radius / std::sin(angle_per_icon);
    }

    bool picker_app::animate(float target, float& value) {
        if(value == target) return true;

        float delta = target / AnimationSpeed;

        if(value < target) {
            value += delta;
            if(value > target) {
                value = target; // clamp to target
            }
        } else if(value > target) {
            value -= delta;
            if(value < target) {
                value = target; // clamp to target
            }
        }

        return false;
    }

    bool picker_app::run_frame() {

        // inspiration: https://github.com/sonnyp/Junction

        w::guard gw{wnd_main};

        if(c.x == 0) {
            ImVec2 wp = ImGui::GetWindowViewport()->WorkPos;
            float cx = wp.x + ImGui::GetWindowWidth() / 2;
            float cy = wp.y + ImGui::GetWindowHeight() / 2;
            c = ImVec2{cx, cy};
        }

        //keyboard_selection_idx = -1;
        //for(int key_index = ImGuiKey_1; key_index <= ImGuiKey_9; key_index++) {
        //    if(ImGui::IsKeyPressed((ImGuiKey)key_index)) {
        //        keyboard_selection_idx = key_index - ImGuiKey_1;
        //        break;
        //    }
        //}

        //if(w::button(ICON_MD_REFRESH)) {
        //    restart_anim();
        //}
        //w::sl();
        w::input(url, "##url", true, -1);

        if(show_extra_menu)
            render_action_menu();
        else
            render_choice_menu();

        // close on Escape key
        if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if(show_extra_menu) {
                show_extra_menu = false;
            } else {
                is_open = false;
            }
        }

        return is_open;
    }

    // Function to get the coordinates of a point on a circle
    void get_point_on_circle(ImVec2 center, float radius, float angle, float& x, float& y) {
        x = center.x + radius * std::cos(angle);
        y = center.y + radius * std::sin(angle);
    }

    void picker_app::render_choice_menu() {
        // draw a dot in the middle of the window

        ImDrawList* dl = ImGui::GetWindowDrawList();
        float dot_radius = DotRadius * app->scale;
        float icon_size = IconRadius * 2.0f * app->scale;
        float active_icon_size = ActiveIconRadius * 2.0f * app->scale;

        ImU32 col_dot = w::imcol32(ImGuiCol_Text);
        ImU32 col_bg = w::imcol32(ImGuiCol_MenuBarBg);

        float angle = 0; // PI is half a circle
        float angle_step = M_PI * 2 / (choices.size() + 1);
        animate(menu_radius, menu_radius_anim);
        animate(inner_radius, inner_radius_anim);
        animate(outer_radius, outer_radius_anim);

        float midpoint = (inner_radius_anim + outer_radius_anim) / 2;
        dl->AddCircle(c, midpoint, col_bg, 0, midpoint);
       
        //dl->AddCircleFilled(c, dot_radius, col_dot);

        // browsers

        for(auto& p : choices) {
            ImVec2 mid_pos;
            get_point_on_circle(c, menu_radius_anim, angle, mid_pos.x, mid_pos.y);
            ImVec2 b_pos = p->ui_is_hovered
                ? ImVec2{mid_pos.x - active_icon_size / 2, mid_pos.y - active_icon_size / 2}
                : ImVec2{mid_pos.x - icon_size / 2, mid_pos.y - icon_size / 2};

            // draw browser icon at the calculated position
            {
                w::group g;
                g.render();
                if(!p->ui_is_hovered) {
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, InactiveAlpha);
                }

                w::cur_set(b_pos);
                float& isz = p->ui_is_hovered ? active_icon_size : p->ui_icon_size_anim;
                w::rounded_image(*app, p->b->get_best_icon_path(), isz, isz, isz / 2);
                bool is_hovered = w::is_hovered();
                bool is_leftclicked = w::is_leftclicked();

                // if required, draw profile icon
                if(!p->is_singular()) {
                    ImVec2 p_pos{mid_pos.x, mid_pos.y};
                    w::cur_set(p_pos);
                    float isz = p->ui_icon2_size_anim;
                    if(p->is_incognito) {
                        w::image(*app, "incognito", isz, isz);
                    } else {
                        w::rounded_image(*app, p->get_best_icon_path(), isz, isz, isz);
                    }
                }

                bool animated = animate(icon_size, p->ui_icon_size_anim);
                animate(icon_size / 2, p->ui_icon2_size_anim);

                if(animated) {
                    // draw label perfectly in the middle
                    ImVec2 wsz = ImGui::CalcTextSize(p->name.c_str());
                    w::cur_set(mid_pos.x - wsz.x / 2, b_pos.y + icon_size);
                    w::label(p->name);
                }

                // draw debug dot
                //dl->AddCircleFilled(mid_pos, dot_radius, col_dot);

                if(!p->ui_is_hovered) {
                    ImGui::PopStyleVar();
                }

                p->ui_is_hovered = is_hovered;
                if(is_hovered) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    if(is_leftclicked) {
                        decision = p;
                        is_open = false;
                    }
                }
            }

            angle += angle_step;
        }

        // menu after angle
        ImVec2 mid_pos;
        get_point_on_circle(c, menu_radius_anim, angle, mid_pos.x, mid_pos.y);
        w::cur_set(mid_pos);
        w::label(ICON_MD_APPS);
        if(w::is_hovered()) {
            w::tooltip("More actions");
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if(w::is_leftclicked()) {
                show_extra_menu = !show_extra_menu;
                restart_anim();
            }
        }
    }

    void picker_app::render_action_menu() {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float angle = 0; // PI is half a circle
        float angle_step = M_PI * 2 / (action_menu_items.size());
        ImU32 col_bg = w::imcol32(ImGuiCol_MenuBarBg);
        dl->AddCircle(c, action_menu_radius_anim, col_bg, 0, 40.0f * app->scale);
        animate(action_menu_radius, action_menu_radius_anim);

        for(auto& item : action_menu_items) {
            ImVec2 mid_pos;
            get_point_on_circle(c, action_menu_radius_anim, angle, mid_pos.x, mid_pos.y);

            ImVec2 wsz = ImGui::CalcTextSize(item.icon.c_str());
            w::cur_set(mid_pos.x - wsz.x / 2, mid_pos.y - wsz.y / 2);
            w::label(item.icon);
            if(w::is_hovered()) {
                w::tooltip(item.tooltip);
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                if(w::is_leftclicked()) {
                    menu_item_clicked(item.id);
                }
            }

            // draw tooltip
            angle += angle_step;
        }
    }

    void picker_app::menu_item_clicked(const std::string& id) {
        if(id == "back") {
            show_extra_menu = !show_extra_menu;
            restart_anim();
        } else if(id == "copy") {
            win32::clipboard::set_ascii_text(url);
            is_open = false;
        } else if(id == "email") {
            win32::clipboard::set_ascii_text(url);
            win32::shell::exec(fmt::format("mailto:?body={}", url), "");
            is_open = false;
        } else if(id == "edit") {

        }
    }

    void picker_app::restart_anim() {
        for(auto& c : choices) {
            c->ui_icon_size_anim = 0.0f;
            c->ui_icon2_size_anim = 0.0f;
            c->b->ui_icon_size_anim = 0.0f;
        }
        menu_radius_anim = 0;
        inner_radius_anim = 0;
        outer_radius_anim = 0;
        action_menu_radius_anim = 0;
    }
}