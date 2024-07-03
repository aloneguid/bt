#include "url_opener.h"
#include "url_pipeline.h"
#include "../globals.h"

namespace bt {
    void url_opener::open(std::shared_ptr<browser_instance> bi, url_payload up) {
        bi->launch(up);
    }

    void url_opener::open(std::shared_ptr<browser_instance> bi, const std::string& url) {
        url_payload up{url};
        open(bi, up);
    }

    void url_opener::open(url_payload up) {
        auto matches = browser::match(g_config.browsers, up);
        browser_match_result& first_match = matches[0];
        up.app_mode = first_match.rule.app_mode;
        open(first_match.bi, up);
    }

    void url_opener::open(const std::string& url) {
        url_payload up{ url };
        open(up);
    }
}