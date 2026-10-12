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
            for(const auto& p : u.parameters) {
                if(str::equal_ic(p.first, "url") || str::equal_ic(p.first, "data")) {
                    const string url = p.second;
                    up.url = str::url_decode(url);
                    return; // Found and processed
                }
            }
        }
    }
}