#pragma once
#include <vector>
#include <memory>
#include "grey.h"
#include "../browser.h"

namespace bt::ui {

    class picker_result {
    public:
        std::shared_ptr<bt::browser_instance> decision;
        std::string url;

        operator bool() const {
            return decision != nullptr;
        }
    };

    class picker_app {
    public:

        struct action_menu_item {
            std::string id;
            std::string icon;
            std::string tooltip;
        };

        picker_app(const std::string& url);
        ~picker_app();

        picker_result run();

        static bool is_hotkey_down();

    private:

        ImVec2 mon_work_pos;
        ImVec2 mon_work_size;

        // flexi sizing
        float padding;
        float icon_size;
        float box_size;
        float pre_menu_height{0.0f};
        ImVec2 window_size;
        float action_button_width{0.0f};

        std::string url;
        std::string title;
        std::unique_ptr<grey::app> app;
        bool is_open{true};
        bool is_settings_open{false};

        std::vector<action_menu_item> action_menu_items{
            action_menu_item{"copy", ICON_MD_CONTENT_COPY, "Copy to clipboard & close"},
            action_menu_item{"email", ICON_MD_EMAIL, "Email link"}
        };

        grey::widgets::window wnd_main;
        grey::widgets::window wnd_settings;
        grey::widgets::container cnt_blist;
        grey::widgets::container cnt_top;
        std::vector<std::shared_ptr<bt::browser_instance>> choices;
        ImU32 clear_color;

        std::shared_ptr<bt::browser_instance> decision;
        int active_idx{0};
        bool url_focused{false};

        bool run_frame();
        void recalc();
        void render_action_menu();
        void render_list();
        void render_settings();
        void menu_item_clicked(const std::string& id);
    };
}