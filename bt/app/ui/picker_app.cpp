#include "picker_app.h"
#include <memory>
#include "../../globals.h"
#include <format>
#include "btwidgets.h"
#include "common/clipboard.h"
#include "common/keyboard.h"

#if PLATFORM_WINDOWS
#include "win32/user.h"
#include "win32/os.h"
#include "win32/shell.h"
#endif

using namespace std;
using namespace grey::common;
namespace w = grey::widgets;

namespace bt::ui {
    picker_app::picker_app(const string& url, std::optional<std::vector<profile_selection> > selections)
        : url{url}, title{APP_LONG_NAME " - Pick"},
          app{grey::app::make(title, 100, 100)},
          wnd_main{title, &is_open},
          wnd_settings{"Settings", &is_settings_open} {
        app->initial_theme_id = g_state.ui_theme;
        app->can_resize = false;
        app->center_on_screen = true;
        app->show_title_bar = g_state.picker.show_native_chrome;

        app->always_on_top = g_state.picker.always_on_top;
#if PLATFORM_WINDOWS
        app->win32_close_on_focus_lost = g_state.picker.close_on_focus_loss;
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
            for(auto& browser: g_state.browsers) {
                if(browser.is_hidden) continue;
                for(size_t i = 0; i < browser.profiles.size(); i++) {
                    if(browser.profiles[i].is_hidden) continue;
                    choices.emplace_back(browser, i);
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
                    .padding(5, 5);

            cnt_blist
                    .background(true);

            wnd_settings
                    .no_collapse()
                    .size(400.0f, 0)
                    .no_scroll()
                    .border(1);
        };
    }

    picker_app::~picker_app() = default;

    picker_result picker_app::run() {
        app->run([this](const grey::app&) {
            return run_frame();
        });

        return picker_result{final_choice, url};
    }

    bool picker_app::is_hotkey_down() {
        // can't check with ImGui, because it's not initialised
        bool k_shift = keyboard::is_kbd_shift_down();
        bool k_ctrl = keyboard::is_kbd_ctrl_down();
        bool k_alt = keyboard::is_kbd_alt_down();
        bool k_caps = keyboard::is_kbd_caps_locks_on();
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
            w::guard gw{wnd_main};

            if(choices.empty()) {
                w::label("no browsers", w::emphasis::error, 0, true, 0, true, true);
            } else {
                ImVec2 cur1 = w::cur_get();

                with_container(cnt_top,
                               render_action_menu();
                );

                ImVec2 cur2 = w::cur_get();
                header_height = cur2.y - cur1.y;

                recalc();

                with_container(cnt_blist,
                               render_list();
                );
            }
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
                    if(active_idx < 0) active_idx = (int) choices.size() - 1;
                }
            }

            // right or down key moves active_idx up
            if(ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                if(!choices.empty()) {
                    active_idx++;
                    if(active_idx >= (int) choices.size()) active_idx = 0;
                }
            }

            // number keys 1-9 change active_idx
            for(int key_index = ImGuiKey_1; key_index <= ImGuiKey_9; key_index++) {
                if(ImGui::IsKeyPressed((ImGuiKey) key_index)) {
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
        ImGuiStyle& style = ImGui::GetStyle();
        box_size_scaled = g_state.picker.box_size * app->scale;
        padding_scaled = g_state.picker.item_padding * app->scale;
        text_size = w::text_size_get("x");
        label_text_size = w::text_size_get("x", g_state.picker.label_size);
        auto url_size = w::text_size_get(url);

        // padding should only be used to space out items, not for any calculations inside
        float max_mon_width = mon_work_size.x * static_cast<float>(g_state.picker.max_width_perc) / 100.0f;
        float max_url_width = url_size.x + action_button_width * (action_menu_items.size() + 2);
        float max_w_width = box_size_scaled * (static_cast<float>(choices.size()) + 1.0f) + style.WindowPadding.x * 2;
        float max_width = max(max_url_width, max_w_width);
        float w_width = min(max_width, max_mon_width);

        float box_size_total = box_size_scaled + padding_scaled * 2;

        // how many items can I fit per w_width?
        items_per_w = static_cast<size_t>(w_width / box_size_total);

        // how many lines do I need to fit all the choices?
        lines_total = static_cast<size_t>(ceil(static_cast<float>(choices.size()) / static_cast<float>(items_per_w)));

        float w_height = style.WindowPadding.y +
                         header_height +
                         box_size_total * lines_total +
                         // padding_scaled +
                         style.WindowPadding.y;
        w_height = min(w_height, mon_work_size.y);

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
            w::point cur = w::cur_get();
            w::button(ICON_MD_SETTINGS "##measure");
            w::sl();
            action_button_width = max_width - w::avail_x();
            w::cur_set(cur);
        }

        float input_width = max_width - static_cast<float>(1 + action_menu_items.size()) * action_button_width;
        w::input(url, "##url", true, input_width);
        url_focused = w::is_focused();

        for(const auto& [id, icon, tooltip]: action_menu_items) {
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

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGuiStyle& style = ImGui::GetStyle();

        // we want items to be centered in the available space
        float box_size_total = box_size_scaled + padding_scaled * 2;
        float icon_size_scaled = box_size_scaled - label_text_size.y * 2;
        float total_w = box_size_total * items_per_w;

        for(size_t i = 0; i < choices.size(); i++) {
            auto& p = choices[i];
            bool is_active = (i == active_idx);
            if(i % items_per_w == 0) {
                float avail_w = w::avail_x();
                if(total_w < avail_w) {
                    float offset = (avail_w - total_w) / 2.0f;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
                }
            } else {
                w::sl();
            }

            if(is_active) {
                draw_list->ChannelsSplit(2);
                draw_list->ChannelsSetCurrent(1);
            }


            {
                w::group g;

                // render icon and come back to starting position
                float x0, y0;
                w::cur_get(x0, y0);
                w::cur_set(x0, y0);
                btw_icon(*app, p, box_size_total / 2 - icon_size_scaled / 2, padding_scaled, icon_size_scaled);

                // draw key highlight
                if(g_state.picker.show_key_hints && i < 9) {
                    string label = format("{}", i + 1);
                    ImVec2 wsz = w::text_size_get(label, g_state.picker.label_size);

                    // Calculate center for the circle
                    ImVec2 circle_center = ImVec2(x0 + box_size_total / 2.0f, y0 + padding_scaled);
                    float radius = max(wsz.x, wsz.y) / 2.0f;

                    // Draw the circle
                    ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
                    ImU32 circle_color = ImGui::GetColorU32(col);
                    ImGui::GetWindowDrawList()->AddCircleFilled(circle_center, radius, circle_color);

                    // label in the middle-top
                    w::cur_set(circle_center.x - wsz.x / 2, circle_center.y - wsz.y / 2);
                    w::label(label, w::emphasis::none, 0, true, g_state.picker.label_size);
                }

                // labels
                ImVec2 min{x0 + padding_scaled / 2, y0 + padding_scaled + icon_size_scaled + style.FramePadding.y};
                ImVec2 max{x0 + box_size_total - padding_scaled / 2, min.y + label_text_size.y * 2};
                float max_width = max.x - min.x;

                w::cur_set(min.x, min.y);

                {
                    w::clip_rect cr{min, max};
                    w::dummy(max.x - min.x, max.y - min.y);
                    string line1 = p.b().name;
                    string line2 = p.profile().name;
                    float line1_width = w::text_size_get(line1, g_state.picker.label_size).x;
                    float line2_width = w::text_size_get(line2, g_state.picker.label_size).x;
                    ImVec2 line1_pos = min;
                    ImVec2 line2_pos = {min.x, min.y + label_text_size.y};
                    if(line1_width < max_width) line1_pos.x += (max_width - line1_width) / 2.0f;
                    if(line2_width < max_width) line2_pos.x += (max_width - line2_width) / 2.0f;

                    {
                        w::font_scaler scaler(g_state.picker.label_size);
                        w::draw_text(line1_pos, w::emphasis::none, p.b().name);
                        w::draw_text(line2_pos, w::emphasis::none, p.profile().name);
                    }
                }

                w::cur_set(x0, y0);
                w::dummy(box_size_total, box_size_total);
            }

            if(w::is_hovered()) {
                active_idx = static_cast<int>(i);
                w::mouse_cursor(w::mouse_cursor_type::hand);
            }

            if(w::is_leftclicked()) {
                final_choice = choices[active_idx];
                is_open = false;
            }

            if(is_active) {
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImU32 bc = ImGui::GetColorU32(ImGuiCol_Border);
                draw_list->ChannelsSetCurrent(0); //background channel
                draw_list->AddRectFilled(min, max, bc, g_state.picker.item_rounding);
                draw_list->ChannelsMerge();
            }
        }

        ImGui::PopStyleVar();
    }

    void picker_app::render_settings() {
        w::combo("icon drawing", {
                     "profile on top of browser",
                     "browser on top of profile",
                     "browser only",
                     "profile only"
                 }, (unsigned int&) g_state.icon_overlay);

#if PLATFORM_WINDOWS
        app->win32_close_on_focus_lost = false; // never close app when settings are open
#endif

        w::slider(g_state.picker.box_size, 5, 256, "item size", 0.1);
        w::slider(g_state.picker.item_padding, 0, 100, "padding", 0.1);
        w::slider(g_state.picker.item_rounding, 0, g_state.picker.box_size / 2, "item rounding", 0.1);
        w::slider(g_state.picker.label_size, -15.0f, 15.0f, "label size", 0.5);
        w::slider(g_state.picker.max_width_perc, 10, 100, "max width %");
        w::checkbox("show key hints (1-9)", g_state.picker.show_key_hints);
        if(w::slider(g_state.picker.border_width, 0, 10, "border width", 1, true)) {
            wnd_main.border(g_state.picker.border_width);
        }
        w::slider(g_state.picker.opacity, 50, 255, "window opacity");
        if(w::checkbox("show native window chrome", g_state.picker.show_native_chrome))
            app->show_title_bar = g_state.picker.show_native_chrome;
        w::tt("When enabled, the window will have standard OS title bar and borders.\nApplies next time picker opens.");

        w::spc(2);
        w::sep();
        if(w::button(ICON_MD_RESTORE " reset", w::emphasis::error)) {
            g_state.picker = picker_state{};
        }
        w::spc(5);
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
