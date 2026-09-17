#pragma once
#include <memory>
#include <string>
#include "grey.h"

namespace bt::ui {

    class viewer_app {
    public:
        viewer_app(const std::string& url);
        void run();

    private:
        std::string url;
        std::unique_ptr<grey::app> app;
        bool is_open{true};
    };
}
