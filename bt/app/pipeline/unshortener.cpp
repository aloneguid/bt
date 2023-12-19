#include "unshortener.h"
#include <map>
#include <set>
#include "url.h"

using namespace std;

namespace bt::pipeline {

    const string LocationHeaderName = "Location";

const set<string> SupportedDomains = {
    "bit.ly",
    "rebrand.ly",
    "bl.ink",
    "zapier.com",
    "short.io",
    "ow.ly",
    "linktr.ee",
    "t2m.io",
    "linkjoy.io",
    "geni.us",
    "pxlme.me",
    "rb.gy",
    "snip.ly",
    "tinyurl.com", "tiny.one",
    "vrch.at"
};

    void unshortener::process(url_payload& up) {

        string url = up.match_url.empty() ? up.url : up.match_url;

        if(!is_supported(url)) return;

        // example: https://bit.ly/47EZHSl -> https://github.com/aloneguid/bt

        map<string, string> headers;
        int code = http.get_get_headers(url, headers);

        map<string, string>::const_iterator it_loc;
        if((code == 301 || code == 302) && (it_loc = headers.find(LocationHeaderName)) != headers.end()) {
            string new_url = it_loc->second;
            up.match_url = up.open_url = new_url;
        }
    }

    bool unshortener::is_supported(const std::string& abs_url) {
        url u{abs_url};
        return SupportedDomains.contains(u.host);
    }
}