#include "replacer.h"
#include "common/str.h"
#include <regex>

using namespace std;
using namespace grey::common;

namespace bt::pipeline {

    replacer::replacer(replacer_kind kind, const std::string& find, const std::string& replace) :
        url_pipeline_step{url_pipeline_step_type::find_replace},
        kind{kind}, find{find}, replace{replace} {
    }

    void replacer::process(click_payload& up) {
        if(kind == replacer_kind::find_replace) {
            size_t idx = up.url.find(find);
            if(idx != string::npos) {
                str::replace_all(up.url, find, replace);
            }
        } else {
            // regex
            regex rgx{find, regex_constants::icase};
            if(regex_search(up.url, rgx)) {
                up.url = regex_replace(up.url, rgx, replace);
            }
        }
    }
}