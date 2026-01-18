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
#include "btwidgets.h"

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
        app->win32_transparent = !g_config.picker_show_native_chrome;
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
            btw_on_app_initialised(*app);

            wnd_main
                //.size(wnd_width, wnd_height_normal)
                .no_titlebar()
                .no_resize()
                .border(g_config.picker_border_width)
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

        float max_url_width = ImGui::CalcTextSize(url.c_str()).x + action_button_width * (action_menu_items.size() + 2);
        float max_w_width = box_size * (choices.size() + 1);
        float max_width = max(max_url_width, max_w_width);

        float w_width = min(max_width, mon_work_size.x);
        float w_height = pre_menu_height + box_size + padding * 2;

        auto target_window_size = ImVec2{w_width, w_height};

        if(window_size.x != target_window_size.x || window_size.y != target_window_size.y) {
            window_size = target_window_size;
            app->resize_main_viewport(window_size.x / app->scale, window_size.y / app->scale);
        }
    }

    void picker_app::render_action_menu() {

        float max_width = w::avail_x();
        {
            // calculate one action button width
            ImVec2 cur = w::cur_get();
            w::button(ICON_MD_SETTINGS "##measure"); w::sl();
            action_button_width = max_width - w::avail_x();
            w::cur_set(cur);
        }

        float input_width = max_width - (1 + action_menu_items.size()) * action_button_width;
        w::input(url, "##url", true, input_width);
        url_focused = w::is_focused();

        if(w::is_hovered()) {
            /*w::sl();
            ImVec2 pos = w::cur_get();
            w::cur_set(ImVec2(pos.x - button_width, pos.y));
            if(w::button(ICON_MD_OPEN_IN_NEW, w::emphasis::secondary, true, true, "expand")) {
                url_popup.open();
            }
            w::mouse_cursor(w::mouse_cursor_type::hand);
            w::cur_set(pos);*/
        }

        for(const action_menu_item& ami : action_menu_items) {
            w::sl();
            if(w::button(ami.icon, w::emphasis::none, true, false, ami.tooltip)) {
                menu_item_clicked(ami.id);
            }
        }

        w::sl();
        if(w::button(ICON_MD_SETTINGS, w::emphasis::none, true, false, "show settings")) {
            is_settings_open = !is_settings_open;
        }
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

                // render icon and come back to starting position
                float x0, y0;
                w::cur_get(x0, y0);

                btw_icon(*app, p, padding, icon_size, is_active);

                // draw key highlight
                if(g_config.picker_show_key_hints && i < 9) {
                    string label = fmt::format("{}", i + 1);
                    ImVec2 wsz = ImGui::CalcTextSize(label.c_str());

                    // label in the middle-top
                    w::cur_set(x0 + box_size / 2.0f - wsz.x / 2, y0);
                    w::label(label.c_str());
                }

                //ImGui::PopStyleVar();

                // draw label
                //w::cur_set(x0 + icon_size + padding * 3, y0 + padding + icon_size / 2 - ImGui::GetTextLineHeight() / 2);
                //w::label(p->get_best_display_name());

               
            }

            if(w::is_hovered()) {
                active_idx = (int)i;
                w::mouse_cursor(w::mouse_cursor_type::hand);
            }

            if(w::is_leftclicked()) {
                decision = choices[active_idx];
                is_open = false;
            }

            w::tt(p->get_best_display_name());
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

        w::combo("icon drawing",
        {
            "profile on top of browser",
            "browser on top of profile",
            "browser only",
            "profile only"},
            (unsigned int&)g_config.icon_overlay);
        w::slider(g_config.picker_icon_size, 5, 256, "icon size");
        w::slider(g_config.picker_item_padding, 0, 100, "padding");
        w::slider(g_config.picker_inactive_item_alpha, 0.1f, 1.0f, "inactive item alpha");
        w::checkbox("show key hints (1-9)", g_config.picker_show_key_hints);
        if(w::slider(g_config.picker_border_width, 0, 5, "border width")) {
            wnd_main.border(g_config.picker_border_width);
        }
        w::checkbox("show native window chrome", g_config.picker_show_native_chrome);
        w::tt("When enabled, the window will have standard OS title bar and borders.\nApplies next time picker opens.");

        if(w::button("reset", w::emphasis::error)) {
            g_config.picker_icon_size = 32;
            g_config.picker_item_padding = 10;
            g_config.picker_inactive_item_alpha = 0.4f;
            g_config.picker_show_key_hints = true;
            g_config.picker_border_width = 1;
            g_config.picker_show_native_chrome = false;
            g_config.icon_overlay = icon_overlay_mode::profile_on_browser;
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