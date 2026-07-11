#pragma once
#include <string>
#include "browser.h"
#include "click_payload.h"

namespace bt {
    class url_opener {
    public:
        static void open(const profile_selection& profile, click_payload up);
        static void open(const profile_selection& profile, const std::string& url);
        static void open(click_payload up);
        static void open(const std::string& url);
    };
}