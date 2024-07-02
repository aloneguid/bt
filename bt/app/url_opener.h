#pragma once
#include <string>
#include <memory>
#include "browser.h"
#include "url_payload.h"

namespace bt {
    class url_opener {
    public:
        static void open(std::shared_ptr<browser_instance> bi, url_payload up);
        static void open(std::shared_ptr<browser_instance> bi, const std::string& url);
        static void open(url_payload up);
        static void open(const std::string& url);
    };
}