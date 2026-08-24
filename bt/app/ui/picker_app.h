#pragma once
#include <vector>
#include <memory>
#include <optional>
#include "grey.h"
#include "../browser.h"

namespace bt::ui {

    class picker_result {
    public:
        std::optional<profile_selection> choice;
        std::string url;

        operator bool() const {
            return static_cast<bool>(choice);
        }
    };

    class picker_app {
    public:

        struct action_menu_item {
            std::string id;
            std::string icon;
            std::string tooltip;
        };

        explicit picker_app(const std::string& url, std::optional<std::vector<profile_selection>> selections = std::nullopt);
        ~picker_app();

        picker_result run();

        /**
         * @brief Check whether a hotkey is currently down. Can be used outside of picker_app to make the appropriate action.
         *
         * @param configured If true (default), the hotkey is checked against the configured hotkey; otherwise, the check is performed for any possible hotkey (useful for configuration UI purposes).
         */
        static bool is_hotkey_down(bool configured = true);

    private:

        ImVec2 mon_work_pos;
        ImVec2 mon_work_size;

        // flexi sizing
        ImVec2 window_size;
        float header_height{0.0f};
        float action_button_width{0.0f};
        float box_size_scaled{0.0f};
        float padding_scaled{0.0f};
        ImVec2 label_text_size{};
        size_t items_per_w{0};
        size_t lines_total{0};

        std::string url;
        std::string title;
        std::unique_ptr<grey::app> app;
        bool is_open{true};
        bool is_settings_open{false};

        std::vector<action_menu_item> action_menu_items{
            action_menu_item{"rule", ICON_MD_RULE, "Toggle rule creator (R)"},
            action_menu_item{"copy", ICON_MD_CONTENT_COPY, "Copy to clipboard & close"},
            action_menu_item{"email", ICON_MD_EMAIL, "Email link"}
        };

        grey::widgets::window wnd_main;
        grey::widgets::window wnd_settings;
        grey::widgets::container cnt_blist;
        grey::widgets::container cnt_top;
        std::vector<profile_selection> choices;
        ImU32 clear_color;

        std::optional<profile_selection> final_choice;
        int active_idx{0};
        bool show_rule_creator{false};
        match_rule creator_rule{};
        bool url_focused{false};

        bool run_frame();
        void recalc();
        void render_action_menu();
        void render_rule_creator();
        void render_list();
        void render_settings();
        void menu_item_clicked(const std::string& id);
        int get_label_line_count();
    };
}