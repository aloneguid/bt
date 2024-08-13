#include "unshortener.h"
#include <map>
#include <set>
#include "url.h"

using namespace std;

namespace bt::pipeline {

    const string LocationHeaderName = "Location";

    const set<string> SupportedDomains = {
        "adf.ly",
        "adfoc.us",
        "bc.vc",
        "bit.ly",
        "bl.ink",
        "geni.us",
        "gg.gg",
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
        "t2m.io",
        "tiny.one",
        "tinyurl.com",
        "vrch.at",
        "zapier.com",
        "zzb.gz"
    };

    void unshortener::process(click_payload& up) {

        if(!is_supported(up.url)) return;

        // example: https://bit.ly/47EZHSl -> https://github.com/aloneguid/bt

        map<string, string> headers;
        int code = http.get_get_headers(up.url, headers);

        map<string, string>::const_iterator it_loc;
        if((code == 301 || code == 302) && (it_loc = headers.find(LocationHeaderName)) != headers.end()) {
            string new_url = it_loc->second;
            up.url = new_url;
        }
    }

    bool unshortener::is_supported(const std::string& abs_url) {
        url u{abs_url};
        return SupportedDomains.contains(u.host);
    }
}