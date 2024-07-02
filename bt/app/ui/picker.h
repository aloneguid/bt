#pragma once
#include <vector>
#include <memory>
#include "grey.h"
#include "../browser.h"

namespace bt::ui {
    class picker_app {
    public:
        picker_app(const std::string& url, std::vector<std::shared_ptr<bt::browser_instance>> choices);
        picker_app(const std::string& url);
        ~picker_app();

        void run();

    private:

        const float BrowserSquareSize = 60.0f;
        const float BrowserSquarePadding = 10.0f;
        const float ProfileSquareSize = 30.0f;
        const float ProfileSquarePadding = 5.0f;
        const float WindowMinWidth = BrowserSquareSize * 6;

        const float InactiveAlpha = 0.8;

        std::string url;
        std::string title;
        std::unique_ptr<grey::app> app;
        bool is_open{true};
        std::vector<std::shared_ptr<bt::browser_instance>> choices;

        // calculated
        std::vector<bt::browser> browsers;
        int active_browser_idx{-1};
        float browser_bar_left_pad{0};

        grey::widgets::window wnd_main;


        bool run_frame();
    };
}