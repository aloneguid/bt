#include "url_opener.h"
#include "url_pipeline.h"
#include "../globals.h"
#include "url.h"

using namespace grey::common;

namespace bt {
    void url_opener::open(std::shared_ptr<browser_instance> bi, click_payload up) {
        bi->launch(up);
    }

    void url_opener::open(std::shared_ptr<browser_instance> bi, const std::string& url) {
        click_payload up{url};
        open(bi, up);
    }

    void url_opener::open(click_payload up) {
        auto matches = browser::match(g_state.browsers, up, g_script);
        if(matches.empty()) {
            url::open_in_browser(up.url);;
        } else {
            browser_match_result& first_match = matches[0];
            up.app_mode = first_match.rule.app_mode;
            open(first_match.bi, up);
        }
    }

    void url_opener::open(const std::string& url) {
        const click_payload up{ url };
        open(up);
    }
}