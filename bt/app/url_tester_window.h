#pragma once
#include "grey.h"
#include "browser.h"
#include "url.h"

namespace bt {
    class url_tester_window : public grey::window {
    public:
        url_tester_window(grey::grey_context& ctx);

    private:
        std::shared_ptr<grey::child> w_matches;
        std::vector<std::shared_ptr<browser_match_result>> matches;
        url_payload up;
        url u{""};

        void match();
        std::string to_icon(match_scope scope);
    };
}