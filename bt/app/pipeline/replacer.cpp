#include "replacer.h"
#include "str.h"
#include <regex>

using namespace std;

namespace bt::pipeline {

    replacer::replacer(replacer_kind kind, const std::string& find, const std::string& replace) :
        url_pipeline_step{url_pipeline_step_type::find_replace},
        kind{kind}, find{find}, replace{replace} {
    }

    replacer::replacer(const std::string& rule) : url_pipeline_step(url_pipeline_step_type::find_replace) {
        // parse rules - each string contains 3 parts separated by a pipe
        // kind|match|replace

        auto parts = str::split_pipe(rule);

        if(parts[0] == "rgx") {
            kind = replacer_kind::regex;
        } else {
            kind = replacer_kind::find_replace;
        }

        if(parts.size() == 3) {
            find = parts[1];
            replace = parts[2];
        }
    }

    std::string replacer::serialise() {
        return str::join_with_pipe({
                        kind == bt::pipeline::replacer_kind::find_replace ? "substr" : "rgx",
                        find,
                        replace});
    }

    void replacer::process(url_payload& up) {
        string url = up.match_url.empty() ? up.url : up.match_url;

        if(kind == replacer_kind::find_replace) {
            size_t idx = url.find(find);
            if(idx != string::npos) {
                str::replace_all(url, find, replace);
                up.match_url = up.open_url = url;
            }
        } else {
            // regex
            regex rgx{find, regex_constants::icase};
            if(regex_search(url, rgx)) {
                up.match_url = up.open_url = regex_replace(url, rgx, replace);
            }
        }
    }
}