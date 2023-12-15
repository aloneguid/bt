#include "replacer.h"
#include "str.h"

using namespace std;

namespace bt::pipeline {

    replacer::replacer(const std::vector<std::string>& rules) : url_pipeline_step(url_pipeline_step_type::replacer) {
        // parse rules - each string contains 3 parts separated by a pipe
        // kind|match|replace

        for(auto& rule : rules) {
            auto parts = str::split_pipe(rule);
            if(parts.size() != 3) {
                continue;
            }

            if(parts[0] == "contains") {
                this->rules.push_back({replacer_kind::contains, parts[1], parts[2]});
            } else if(parts[0] == "regex") {
                this->rules.push_back({replacer_kind::regex, parts[1], parts[2]});
            }
        }
    }

    void replacer::process(url_payload& up) {
        string url = up.match_url.empty() ? up.url : up.match_url;

        for(auto& rule : rules) {
            if(rule.kind == replacer_kind::contains) {
                size_t idx = url.find(rule.match);
                if(idx != string::npos) {
                    str::replace_all(url, rule.match, rule.replace);
                    up.match_url = up.open_url = url;
                }
            }
        }
    }
}