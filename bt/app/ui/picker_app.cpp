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
        app{grey::app::make(title, 100, 100)},
        wnd_main{title, &is_open},
        wnd_settings{"Settings", &is_settings_open} {

        app->initial_theme_id = g_config.theme_id;
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
                //.no_background()
                .no_scroll();

            cnt_top
                .auto_size_y()
                .padding(10, 10);

            cnt_blist
                .background(true)
                .horizontal_scrollbar();

            wnd_settings
                .no_collapse()
                .size(400.0f, 0)
                .no_scroll()
                .border(1);
        };

    }

    picker_app::~picker_app() {
        g_config.commit();
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


        // get monitor dimensions
        int mon_idx = app->find_monitor_for_main_viewport();
        if(mon_idx != -1) {
            ImGuiPlatformIO io = ImGui::GetPlatformIO();
            ImGuiPlatformMonitor monitor = io.Monitors[mon_idx];
            mon_work_pos = monitor.WorkPos;
            mon_work_size = monitor.WorkSize;
        }

        {
            // window padding off for full use of space and more correct size calculations
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

            w::guard gw{wnd_main};

            ImVec2 cur = w::cur_get();

            with_container(cnt_top,
                render_action_menu();
            );

            ImGui::Spacing();

            if(pre_menu_height == 0.0f) {
                ImVec2 cur1 = w::cur_get();
                //float y = ImGui::GetWindowViewport()->WorkPos.y;
                //pre_menu_height = cur.y - y;
                pre_menu_height = cur1.y - cur.y;

                // add ImGui element spacing
                pre_menu_height += ImGui::GetStyle().ItemSpacing.y * 2;
            }

            recalc();

            with_container(cnt_blist,
                render_list();
            );
            ImGui::PopStyleVar();
        }

        if(is_settings_open) {
            w::guard gw{wnd_settings};
            render_settings();
        }

        if(!url_focused) {

            // close on Escape key
            if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                is_open = false;
            }

            // left or up key moves active_idx down
            if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                if(!choices.empty()) {
                    active_idx--;
                    if(active_idx < 0) active_idx = (int)choices.size() - 1;
                }
            }

            // right or down key moves active_idx up
            if(ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                if(!choices.empty()) {
                    active_idx++;
                    if(active_idx >= (int)choices.size()) active_idx = 0;
                }
            }

            // number keys 1-9 change active_idx
            for(int key_index = ImGuiKey_1; key_index <= ImGuiKey_9; key_index++) {
                if(ImGui::IsKeyPressed((ImGuiKey)key_index)) {
                    active_idx = key_index - ImGuiKey_1;
                    decision = choices[active_idx];
                    is_open = false;
                    break;
                }
            }
        }

        // invoke action on Enter
        if(ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if(active_idx >= 0 && active_idx < choices.size()) {
                decision = choices[active_idx];
                is_open = false;
            }
        }

        //ImGui::ShowDemoWindow();

        return is_open;
    }

    void picker_app::recalc() {
        padding = g_config.picker_item_padding * app->scale;
        icon_size = g_config.picker_icon_size * app->scale;
        box_size = padding + icon_size + padding;

        float max_w_width = box_size * (choices.size() + 1);
        float w_width = min(max_w_width, mon_work_size.x);
        float w_height = pre_menu_height + box_size + padding * 2;

        auto target_window_size = ImVec2{w_width, w_height};

        if(window_size.x != target_window_size.x || window_size.y != target_window_size.y) {
            window_size = target_window_size;
            app->resize_main_viewport(window_size.x / app->scale, window_size.y / app->scale);
        }
    }

    void picker_app::render_action_menu() {

        if(w::button(ICON_MD_SETTINGS, w::emphasis::none, true, false, "show settings")) {
            is_settings_open = !is_settings_open;
        }

        for(const action_menu_item& ami : action_menu_items) {
            w::sl();
            if(w::button(ami.icon, w::emphasis::none, true, false, ami.tooltip)) {
                menu_item_clicked(ami.id);
            }
        }

        w::sl();
        w::input(url, "##url", true, -FLT_MIN);
        url_focused = ImGui::IsItemFocused();
    }

    void picker_app::render_list() {
        // spacing needs to be turned off for list to avoid gaps horizontally
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        // we want items to be centered in the available space
        float total_w = box_size * choices.size();
        float avail_w = ImGui::GetContentRegionAvail().x;
        if(total_w < avail_w) {
            float offset = (avail_w - total_w) / 2.0f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
        }

        for(size_t i = 0; i < choices.size(); i++) {
            auto& p = choices[i];
            bool is_active = (i == active_idx);
            if(i > 0) w::sl();

            {
                w::group g;
                g.render();

                // render icon and come back to starting position
                float x0, y0;
                w::cur_get(x0, y0);

                // dummy
                ImGui::Dummy(ImVec2{box_size, box_size});

                w::cur_set(x0 + padding, y0 + padding);

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, is_active ? 1 : g_config.picker_inactive_item_alpha);

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

                // draw key highlight
                if(g_config.picker_show_key_hints && i < 10) {
                    string label = fmt::format("{}", i + 1);
                    ImVec2 wsz = ImGui::CalcTextSize(label.c_str());

                    // label in the middle-top
                    w::cur_set(x0 + box_size / 2.0f - wsz.x / 2, y0);
                    w::label(label.c_str());
                }

                ImGui::PopStyleVar();

                // draw label
                //w::cur_set(x0 + icon_size + padding * 3, y0 + padding + icon_size / 2 - ImGui::GetTextLineHeight() / 2);
                //w::label(p->get_best_display_name());

               
            }

            if(w::is_hovered()) {
                active_idx = (int)i;
            }

            if(w::is_leftclicked()) {
                decision = choices[active_idx];
                is_open = false;
            }

            w::tooltip(p->get_best_display_name());
        }

        ImGui::PopStyleVar();
    }

    void picker_app::render_settings() {
        //ImGuiViewport* vp = ImGui::GetMainViewport();

        //w::label(fmt::format("monitor wp: {}x{}, ws: {}x{}", mon_work_pos.x, mon_work_pos.y, mon_work_size.x, mon_work_size.y));
        //w::label(fmt::format("viewport wp {}x{}, ws: {}x{}", vp->WorkPos.x, vp->WorkPos.y, vp->WorkSize.x, vp->WorkSize.y));
        //w::label(fmt::format("window_size: {}x{}", window_size.x, window_size.y));

        //ImVec2 pos = w::cur_get();
        //w::label(fmt::format("cursor pos: {}x{}", pos.x, pos.y));
        //w::label(fmt::format("box size: {}, pmh: {}", box_size, pre_menu_height));

        w::slider(g_config.picker_icon_size, 5, 256, "icon size");
        w::slider(g_config.picker_item_padding, 0, 100, "padding");
        w::slider(g_config.picker_inactive_item_alpha, 0.1f, 1.0f, "inactive item alpha");
        w::checkbox("show key hints (1-9)", g_config.picker_show_key_hints);

        if(w::button("reset", w::emphasis::error)) {
            g_config.picker_icon_size = 32;
            g_config.picker_item_padding = 10;
            g_config.picker_inactive_item_alpha = 0.4f;
            g_config.picker_show_key_hints = true;
        }

//#if _DEBUG
//        w::sl();
//        if(w::button("double items")) {
//            const size_t c = choices.size();                  // snapshot current count
//            choices.reserve(choices.size() + c);              // avoid reallocation (keeps iterators valid)
//            choices.insert(choices.end(), choices.begin(), choices.begin() + c); // append first c items
//        }
//#endif

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