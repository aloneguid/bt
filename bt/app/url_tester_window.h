#pragma once
#include "grey.h"
#include "browser.h"

namespace bt {
    class url_tester_window : public grey::window {
    public:
        url_tester_window(grey::grey_context& ctx);

    private:
        std::shared_ptr<grey::complex_table<browser_match_result>> tbl;
        std::vector<std::shared_ptr<browser_match_result>> matches;
        std::string proto;
        std::string host;
        std::string path;

        void match(const std::string& s);
        std::string to_icon(match_scope scope);
    };
}