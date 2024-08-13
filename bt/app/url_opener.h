#pragma once
#include <string>
#include <memory>
#include "browser.h"
#include "click_payload.h"

namespace bt {
    class url_opener {
    public:
        static void open(std::shared_ptr<browser_instance> bi, click_payload up);
        static void open(std::shared_ptr<browser_instance> bi, const std::string& url);
        static void open(click_payload up);
        static void open(const std::string& url);
    };
}