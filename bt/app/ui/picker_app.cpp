#include "picker_app.h"
#include <memory>
#include "../../globals.h"
#include <format>
#include "btwidgets.h"
#include "common/clipboard.h"

#if PLATFORM_WINDOWS
#include "win32/user.h"
#include "win32/os.h"
#include "win32/shell.h"
#endif

using namespace std;
using namespace grey::common;
namespace w = grey::widgets;

namespace bt::ui {
    picker_app::picker_app(const string& url, std::optional<std::vector<profile_selection>> selections)
        : url{url}, title{APP_LONG_NAME " - Pick"},
        app{grey::app::make(title, 100, 100)},
        wnd_main{title, &is_open},
        wnd_settings{"Settings", &is_settings_open} {

        app->initial_theme_id = g_state.ui_theme;
        app->can_resize = false;
        app->center_on_screen = true;

#if PLATFORM_WINDOWS
        app->win32_close_on_focus_lost = g_state.picker.close_on_focus_loss;
        app->win32_always_on_top = g_state.picker.always_on_top;
        app->win32_title_bar = g_state.picker.show_native_chrome;
#endif
        auto cc = app->get_clear_color();
        ImU32 cc1 = w::rgb_colour{ImVec4(cc[0], cc[1], cc[2], cc[3])};
        clear_color = cc1;

        // process URL with pipeline
        {
            click_payload up{url};
            g_pipeline.process(up);
            this->url = up.url;
        }

        if(selections) {
            choices = *selections;
        } else {
            choices.clear();
            for(auto& browser : g_state.browsers) {
                if(browser.is_hidden) continue;
                for(size_t i = 0; i < browser.profiles.size(); i++) {
                    if(browser.profiles[i].is_hidden) continue;
                    choices.push_back(profile_selection{browser, i});
                }
            }
        }

        app->on_initialised = [this]() {
            btw_on_app_initialised(*app);

            wnd_main
                .no_titlebar()
                .no_resize()
                .border(static_cast<float>(g_state.picker.border_width))
                .fill_viewport()
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
    }

    picker_result picker_app::run() {
        app->run([this](const grey::app&) {
            return run_frame();
        });

        return picker_result{final_choice, url};
    }

    bool picker_app::is_hotkey_down() {
        ImGuiIO& io = ImGui::GetIO();
        bool k_shift = io.KeyShift;
        bool k_ctrl = io.KeyCtrl;
        bool k_alt = io.KeyAlt;
        // ImGui doesn't have a reliable cross-platform way to check Caps Lock toggle state,
        // using IsKeyDown as a best-effort, though it checks if key is physically held.
        bool k_caps = ImGui::IsKeyDown(ImGuiKey_CapsLock);

        return
            (g_state.picker.invoke.on_key_alt_shift && (k_alt && k_shift)) ||
            (g_state.picker.invoke.on_key_control_alt && (k_ctrl && k_alt)) ||
            (g_state.picker.invoke.on_key_control_shift && (k_ctrl && k_shift)) ||
            (g_state.picker.invoke.on_key_caps_locks && k_caps);
    }

    bool picker_app::run_frame() {

#if PLATFORM_WINDOWS
        app->win32_transparency_window_alpha = g_state.picker.opacity;
#endif

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

            if(choices.empty()) {
                w::label("no browsers", w::emphasis::error);
            } else {

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
            }
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
                    final_choice = choices[active_idx];
                    is_open = false;
                    break;
                }
            }
        }

        // invoke action on Enter
        if(ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if(active_idx >= 0 && active_idx < choices.size()) {
                final_choice = choices[active_idx];
                is_open = false;
            }
        }

        g_config.tick(ImGui::GetIO().DeltaTime);

        return is_open;
    }

    void picker_app::recalc() {
        padding = g_state.picker.item_padding * app->scale;
        icon_size = g_state.picker.icon_size * app->scale;
        box_size = padding + icon_size + padding;
        float max_mon_width = mon_work_size.x * static_cast<float>(g_state.picker.max_width_perc) / 100.0f;

        float max_url_width = ImGui::CalcTextSize(url.c_str()).x + action_button_width * (action_menu_items.size() + 2);
        float max_w_width = box_size * (static_cast<float>(choices.size()) + 1.0f);
        float max_width = max(max_url_width, max_w_width);

        float w_width = min(max_width, max_mon_width);
        float w_height = pre_menu_height + box_size + padding * 2;
        if(w_width < max_width) {
            // compensate for scrollbar
            w_height += ImGui::GetStyle().ScrollbarSize;
        }

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

        float input_width = max_width - static_cast<float>(1 + action_menu_items.size()) * action_button_width;
        w::input(url, "##url", true, input_width);
        url_focused = w::is_focused();

        for(const auto& [id, icon, tooltip] : action_menu_items) {
            w::sl();
            if(w::button(icon, w::emphasis::none, true, false, tooltip)) {
                menu_item_clicked(id);
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
                if(g_state.picker.show_key_hints && i < 9) {
                    string label = format("{}", i + 1);
                    ImVec2 wsz = ImGui::CalcTextSize(label.c_str());

                    // Calculate center for the circle
                    ImVec2 circle_center = ImVec2(x0 + box_size / 2.0f, y0 + wsz.y / 2.0f);
                    float radius = max(wsz.x, wsz.y) / 2.0f;

                    // Draw the circle
                    ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
                    ImU32 circle_color = ImGui::GetColorU32(col);
                    ImGui::GetWindowDrawList()->AddCircleFilled(circle_center, radius, circle_color);

                    // label in the middle-top
                    w::cur_set(x0 + box_size / 2.0f - wsz.x / 2, y0);
                    w::label(label, w::emphasis::none, 0, true);
                }
            }

            if(w::is_hovered()) {
                active_idx = static_cast<int>(i);
                w::mouse_cursor(w::mouse_cursor_type::hand);
            }

            if(w::is_leftclicked()) {
                final_choice = choices[active_idx];
                is_open = false;
            }

            w::tt(p.b().get_best_display_name(p.profile()));
        }

        ImGui::PopStyleVar();
    }

    void picker_app::render_settings() {

        w::combo("icon drawing", {
            "profile on top of browser",
            "browser on top of profile",
            "browser only",
            "profile only"
        }, (unsigned int&)g_state.icon_overlay);

#if PLATFORM_WINDOWS
        app->win32_close_on_focus_lost = false; // never close app when settings are open
#endif

        w::slider(g_state.picker.icon_size, 5, 256, "icon size");
        w::slider(g_state.picker.item_padding, 0, 100, "padding");
        w::slider(g_state.picker.inactive_item_alpha, 0.1f, 1.0f, "inactive item alpha");
        w::slider(g_state.picker.max_width_perc, 10, 100, "max width %");
        w::checkbox("show key hints (1-9)", g_state.picker.show_key_hints);
        if(w::slider(g_state.picker.border_width, 0, 10, "border width", 1, true)) {
            wnd_main.border(g_state.picker.border_width);
        }
        w::slider(g_state.picker.opacity, 50, 255, "window opacity");
        w::checkbox("show native window chrome", g_state.picker.show_native_chrome);
        w::tt("When enabled, the window will have standard OS title bar and borders.\nApplies next time picker opens.");

        if(w::button("reset", w::emphasis::error)) {
            g_state.picker = picker_state{};
        }

        w::spc(3);
    }

    void picker_app::menu_item_clicked(const std::string& id) {
        if(id == "copy") {
            clipboard::set_text(url);
            is_open = false;
        } else if(id == "email") {
            clipboard::set_text(url);
#if PLATFORM_WINDOWS
            win32::shell::exec(format("mailto:?body={}", url), "");
#endif
            is_open = false;
        } else if(id == "edit") {

        }
    }
}