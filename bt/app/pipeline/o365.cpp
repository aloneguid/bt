#include "o365.h"
#include "url.h"
#include "str.h"

using namespace std;

namespace bt::pipeline {
    void o365::process(click_payload& up) {
        url u{up.url};

        if(u.host.ends_with(".safelinks.protection.outlook.com") ||
            u.host == "statics.teams.cdn.office.net") {
            for(const auto& p : u.parameters) {
                if(p.first == "url") {
                    string url = p.second;
                    up.url = str::url_decode(url);
                }
            }
        }
    }
}