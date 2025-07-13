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

        const float WindowSize{400.f};
        const float IconRadius{20.0f};
        const float ActiveIconRadius{22.0f};
        const float IconPadding{15.0f};
        const float DotRadius{3.0f};
        const float AnimationSpeed{10.0f};
        const float InactiveAlpha{0.8f};

        std::string url;
        std::string title;
        std::unique_ptr<grey::app> app;
        bool is_open{true};

        std::vector<action_menu_item> action_menu_items{
            action_menu_item{"back", ICON_MD_MORE, "Back to selection"},
            action_menu_item{"copy", ICON_MD_CONTENT_COPY, "Copy to clipboard & close"},
            action_menu_item{"email", ICON_MD_EMAIL, "Email link"}
        };

        grey::widgets::window wnd_main;
        std::vector<std::shared_ptr<bt::browser_instance>> choices;
        ImVec2 c;
        float menu_radius{0.0f};
        float inner_radius{0.0f};
        float outer_radius{0.0f};
        float menu_radius_anim{0.0f};
        float inner_radius_anim{0.0f};
        float outer_radius_anim{0.0f};
        bool show_extra_menu{false};
        float action_menu_radius{0.0f};
        float action_menu_radius_anim{0.0f};

        std::shared_ptr<bt::browser_instance> decision;

        float get_circle_radius_for_icons(int icon_count, float icon_radius);
        bool animate(float target, float& speed);
        bool run_frame();
        void render_choice_menu();
        void render_action_menu();
        void menu_item_clicked(const std::string& id);
        void restart_anim();
    };
}