#pragma once
#include "grey.h"

namespace bt {
    class about_window : public grey::window {
    public:
        about_window(grey::grey_context& ctx);
    };
}