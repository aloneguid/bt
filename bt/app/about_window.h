#pragma once
#include <memory>
#include "grey.h"

namespace bt {
    class about_window : public grey::window {
    public:
        explicit about_window(grey::grey_context& ctx);

    private:
        grey::grey_context& ctx;
        std::shared_ptr<grey::label> st_fps;
        std::string mem_display;
        std::string cpu_display;

        void refresh_system_status();
    };
}