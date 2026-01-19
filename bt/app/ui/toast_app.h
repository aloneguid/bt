#pragma once
#include <memory>
#include "grey.h"
#include "../browser.h"
#include "../click_payload.h"

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

        toast_app(const click_payload& cp, std::shared_ptr<bt::browser_instance> bi);

        void run();

    private:
        ImVec2 mon_work_pos;
        ImVec2 mon_work_size;

        //const float EdgePadding = 10.0f;
        const float AnimSpeed = 0.2f; // between 0 and 1,

        anim_stage stage{anim_stage::init};
        const click_payload& cp;
        std::string line1;
        std::shared_ptr<bt::browser_instance> bi;
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