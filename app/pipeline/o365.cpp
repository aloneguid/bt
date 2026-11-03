#include "o365.h"
#include "common/url.h"
#include "common/str.h"

using namespace std;
using namespace grey::common;

namespace bt::pipeline {
    void o365::process(click_payload& up) {
        url u{up.url};
        string host = u.host;
        str::lower(host);

        if(host.ends_with(".safelinks.protection.outlook.com") ||
            host == "safelinks.protection.outlook.com" ||
            host == "statics.teams.cdn.office.net" ||
            host == "teams.public.onecdn.static.microsoft") {

            const string* fallback_url = nullptr;

            for(const auto& p : u.parameters) {
                if(str::equal_ic(p.first, "url")) {
                    up.url = str::url_decode(p.second);
                    return;
                }
                if(str::equal_ic(p.first, "data") && !fallback_url) {
                    fallback_url = &p.second;
                }
            }

            if(fallback_url) {
                up.url = str::url_decode(*fallback_url);
            }
        }
    }
}
