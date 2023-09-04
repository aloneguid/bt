#include "o365.h"
#include "url.h"
#include "str.h"

using namespace std;

namespace bt::pipeline {
    void o365::process(url_payload& up) {
        url u{up.url};

        if(u.host.ends_with(".safelinks.protection.outlook.com")) {
            for(const auto& p : u.parameters) {
                if(p.first == "url") {
                    string url = p.second;
                    url = str::url_decode(url);
                    up.match_url = url;
                    up.open_url = up.url;
                }
            }
        }
    }
}