#include "unshortener.h"
#include <map>
#include <set>
#include "common/url.h"
#include "common/str.h"

using namespace std;
using namespace grey::common;

namespace bt::pipeline {

    const string LocationHeaderName = "Location";

    const set<string> SupportedDomains = {
        "adf.ly",
        "adfoc.us",
        "bc.vc",
        "bit.ly",
        "bl.ink",
        "cutt.ly",
        "geni.us",
        "gg.gg",
        "is.gd",
        "linkjoy.io",
        "linktr.ee",
        "ow.ly",
        "ouo.io",
        "pxlme.me",
        "rb.gy",
        "rebrand.ly",
        "short.io",
        "shorte.st",
        "shorturl.at",
        "snip.ly",
        "t.co",
        "t2m.io",
        "tiny.one",
        "tinyurl.com",
        "v.gd",
        "vrch.at",
        "zapier.com",
        "zzb.gz"
    };

    void unshortener::process(click_payload& up) {

        if(!is_supported(up.url)) return;

        // example: https://bit.ly/47EZHSl -> https://github.com/aloneguid/bt

        map<string, string> headers;
        int code = h.get_get_headers(up.url, headers);

        // handle standard HTTP redirects (301, 302, 303, 307, 308)
        if(code == 301 || code == 302 || code == 303 || code == 307 || code == 308) {
            for(const auto& [name, value] : headers) {
                if(str::equal_ic(name, LocationHeaderName)) {
                    up.url = value;
                    break;
                }
            }
        }
    }

    bool unshortener::is_supported(const std::string& abs_url) {
        url u{abs_url};
        string host = u.host;
        str::lower(host);
        return SupportedDomains.contains(host);
    }
}