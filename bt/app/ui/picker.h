#pragma once
#include <vector>
#include <memory>
#include "grey.h"
#include "../browser.h"

namespace bt::ui {
    class picker_app {
    public:
        picker_app(std::vector<std::shared_ptr<bt::browser_instance>> choices);
        picker_app();
        ~picker_app();

        void run();

    private:

        const float BrowserIconSize = 50.0f;
        const float ProfileIconSize = 40.0f;
        const float InactiveAlpha = 0.9;

        std::string title;
        std::unique_ptr<grey::app> app;
        bool is_open{true};
        std::vector<std::shared_ptr<bt::browser_instance>> choices;

        // calculated
        std::vector<bt::browser> browsers;

        grey::widgets::window wnd_main;


        bool run_frame();
    };
}