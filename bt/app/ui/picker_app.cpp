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
        app{grey::app::make(title, WindowSize, WindowSize)}, wnd_main{title, &is_open} {
        app->initial_theme_id = g_config.theme_id;
        //app->load_icon_font = false;
        app->win32_can_resize = false;
        app->win32_center_on_screen = true;
        app->win32_close_on_focus_lost = g_config.picker_close_on_focus_loss;
        app->win32_always_on_top = g_config.picker_always_on_top;
        app->win32_transparent = true;
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

        min_choices = choices.size() + action_menu_items.size();
        if(min_choices < MinChoices) min_choices = MinChoices; // minimum 5 choices to show
        menu_radius = get_circle_radius_for_icons(min_choices, (IconRadius + IconPadding) * app->scale);
        inner_radius = menu_radius - (IconRadius * app->scale);
        outer_radius = menu_radius + (IconRadius * app->scale);
        
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

            float max_size = 
                menu_radius * 2 +
                IconRadius * 2 * app->scale +
                IconPadding * 2 * app->scale;
            app->resize_main_viewport(max_size, max_size);

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
        if(g_config.picker_show_url) {
            w::cur_set(
                c.x - menu_radius,
                c.y - menu_radius - IconRadius * 4 * app->scale);
            w::input(url, "##url", true, menu_radius * 2);
        }

        render_choice_menu();

        // close on Escape key
        if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            is_open = false;
        }

        // move selections with arrow keys
        int max = choices.size() + action_menu_items.size();
        if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            if(active_idx >= 0) {
                active_idx--;
                if(active_idx < 0) active_idx = max - 1;
            }
        } else if(ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            if(active_idx <= max) {
                active_idx++;
                if(active_idx > max) active_idx = 0;
            }
        }

        // invoke action on Enter
        if(ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if(active_idx >= 0 && active_idx < choices.size()) {
                decision = choices[active_idx];
                is_open = false;
            } else if(active_idx >= choices.size() && active_idx < max) {
                // action menu
                auto& item = action_menu_items[active_idx - choices.size()];
                menu_item_clicked(item.id);
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
        float icon_padding = IconPadding * app->scale;
        float active_icon_size = ActiveIconRadius * 2.0f * app->scale;

        ImU32 col_dot = w::imcol32(ImGuiCol_Text);
        ImU32 col_bg = w::imcol32(ImGuiCol_ScrollbarBg);
        ImU32 col_bg1 = w::imcol32(ImGuiCol_ScrollbarGrab);

        float angle = 0; // PI is half a circle
        float angle_step = M_PI * 2 / min_choices;
        animate(menu_radius, menu_radius_anim);
        animate(inner_radius, inner_radius_anim);
        animate(outer_radius, outer_radius_anim);

        // background ring and main ring on top
        dl->AddCircle(c, menu_radius + icon_padding, col_bg1, 0, icon_size + icon_padding * 2);
        dl->AddCircle(c, menu_radius, col_bg, 0, icon_size + icon_padding * 2);

        // segmentation divider lines
        //float line_angle = angle_step / 2; // start from the middle of the first segment
        //for(int i = 0; i < min_choices; i++, line_angle += angle_step) {
        //    ImVec2 to;
        //    get_point_on_circle(c, outer_radius_anim * 2, line_angle, to.x, to.y);
        //    dl->AddLine(c, to, clear_color, 2 * app->scale);
        //}
       
        //dl->AddCircleFilled(c, dot_radius, col_dot);

        // choices

        int idx = 0;
        for(auto& p : choices) {
            ImVec2 mid_pos;
            get_point_on_circle(c, menu_radius_anim, angle, mid_pos.x, mid_pos.y);
            bool is_active = (active_idx == idx);
            ImVec2 b_pos = is_active
                ? ImVec2{mid_pos.x - active_icon_size / 2, mid_pos.y - active_icon_size / 2}
                : ImVec2{mid_pos.x - icon_size / 2, mid_pos.y - icon_size / 2};

            // draw browser icon at the calculated position
            {
                w::group g;
                g.render();
                if(!is_active) {
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, InactiveAlpha);
                } else {
                    //float angle_from = angle - angle_step / 2;
                    //float angle_to = angle + angle_step / 2;
                    //// draw selected segment in different background
                    //dl->PathArcTo(c, outer_radius_anim,
                    //    angle_from,
                    //    angle_to);
                    //dl->PathLineTo(c);
                    //ImVec2 to;
                    //get_point_on_circle(c, outer_radius_anim, angle_from, to.x, to.y);
                    //dl->PathLineTo(to);
                    //dl->PathStroke(clear_color, 0, 2 * app->scale);
                }

                w::cur_set(b_pos);
                float& isz = is_active ? active_icon_size : p->ui_icon_size_anim;
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

                if(!is_active) {
                    ImGui::PopStyleVar();
                }

                if(is_hovered) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    active_idx = idx;
                    if(is_leftclicked) {
                        decision = p;
                        is_open = false;
                    }
                }
            }

            angle += angle_step;
            idx++;
        }

        idx = choices.size();
        for(auto& item : action_menu_items) {
            bool is_active = (active_idx == idx);
            ImVec2 mid_pos;
            get_point_on_circle(c, menu_radius_anim, angle, mid_pos.x, mid_pos.y);

            ImVec2 wsz = ImGui::CalcTextSize(item.icon.c_str());
            w::cur_set(mid_pos.x - wsz.x / 2, mid_pos.y - wsz.y / 2);
            w::label(item.icon, is_active ? w::emphasis::primary : w::emphasis::none);
            if(w::is_hovered()) {
                w::tooltip(item.tooltip);
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                if(w::is_leftclicked()) {
                    menu_item_clicked(item.id);
                }
            }

            // draw tooltip
            angle += angle_step;
            idx++;
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

    void picker_app::restart_anim() {
        for(auto& c : choices) {
            c->ui_icon_size_anim = 0.0f;
            c->ui_icon2_size_anim = 0.0f;
            c->b->ui_icon_size_anim = 0.0f;
        }
        menu_radius_anim = 0;
        inner_radius_anim = 0;
        outer_radius_anim = 0;
    }

    void picker_app::select(std::shared_ptr<bt::browser_instance> bi) {
        decision = bi;
        for(auto& c : choices) {
            //c->ui_is_selected = (c == bi);
        }
    }
}