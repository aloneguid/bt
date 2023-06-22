#pragma once
#include "grey.h"
#include "setup.h"

namespace bt {
    class dash_window : public grey::window {
    public:
        dash_window(grey::grey_context& ctx);

        bool recheck();

    private:
        std::vector<system_check> checks;
    };
}