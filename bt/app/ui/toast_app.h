#pragma once
#include <memory>
#include "grey.h"

namespace bt::ui {

    class toast_app {
    public:

        enum class anim_stage {
            init,
            expand,
            show,
            shrink,
            exit
        };

        toast_app(const std::string& text);

        void run();

    private:
        //const float EdgePadding = 10.0f;
        const float AnimSpeed = 0.2f; // between 0 and 1,
        const float ShowDuration = 2.0f; // seconds

        anim_stage stage{anim_stage::init};
        std::string text;
        ImVec2 wnd_size{0, 0};
        ImVec2 wnd_size_anim{0, 0};
        ImVec2 mon_mid{0, 0};
        float icon_size;
        float show_timer{0.0f};

        std::unique_ptr<grey::app> app;
        bool is_open{true};
        grey::widgets::window wnd_main;

        void size_to_fit();
        void render_content();
    };
}