#pragma once
#include "grey.h"
#include "browser.h"

namespace bt {
    class url_tester_window : public grey::window {
    public:
        url_tester_window(grey::grey_context& ctx);

    private:
        std::vector<std::shared_ptr<browser_match_result>> matches;
        std::string proto;
        std::string host;
        std::string path;
    };
}