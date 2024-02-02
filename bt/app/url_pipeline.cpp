#include "url_pipeline.h"
#include "pipeline/unshortener.h"
#include "pipeline/o365.h"
#include "pipeline/replacer.h"
#include "../globals.h"
#include "str.h"

using namespace std;

namespace bt {

    url_pipeline::url_pipeline(config& cfg) : cfg{cfg} {
        load();
    }

    void url_pipeline::process(url_payload& up) {

        // remove custom protocol prefix
        if(up.url.starts_with(CustomProtoName) && up.url.size() > CustomProtoName.size() + 3) {
            up.url = up.url.substr(CustomProtoName.size() + 3);
        }

        // Firefox for some reason removes ':' when opening custom protocol links, so we need to add it back
        size_t idx = up.url.find("://");
        if(idx == string::npos) {
            idx = up.url.find("//");
            if(idx != string::npos) {
                up.url = up.url.substr(0, idx) + ":" + up.url.substr(idx);
            }
        }

        for(auto& step : steps) {
            step->process(up);
        }

        if(up.match_url.empty())
            up.match_url = up.url;

        if(up.open_url.empty())
            up.open_url = up.url;
    }

    void url_pipeline::load() {
        steps.clear();

        vector<string> steps_str = cfg.get_pipeline();
        for(const string& step_str : steps_str) {
            vector<string> parts = str::split_pipe(step_str);
            if(parts.size() < 1)
                continue;

            string name = parts[0];

            if(name == "o365") {
                steps.push_back(make_shared<bt::pipeline::o365>());
            } else if(name == "unshorten") {
                steps.push_back(make_shared<bt::pipeline::unshortener>());
            } else if(name == "find_replace") {
                if(parts.size() < 4)
                    continue;
                steps.push_back(make_shared<bt::pipeline::replacer>(
                    parts[1] == "rgx"
                    ? bt::pipeline::replacer_kind::regex : bt::pipeline::replacer_kind::find_replace,
                    parts[2],
                    parts[3]));
            }
        }
    }

    void url_pipeline::save() {
        // convert steps to string vector
        vector<string> steps_str;
        for(auto step : steps) {
            switch(step->type) {
                case url_pipeline_step_type::o365:
                    steps_str.push_back("o365");
                    break;
                case url_pipeline_step_type::unshortener:
                    steps_str.push_back("unshorten");
                    break;
                case url_pipeline_step_type::find_replace:
                    std::shared_ptr<bt::pipeline::replacer> replacer_step = std::static_pointer_cast<bt::pipeline::replacer>(step);                    
                    steps_str.push_back(str::join_with_pipe({
                        "find_replace",
                        replacer_step->kind == bt::pipeline::replacer_kind::find_replace ? "substr" : "rgx",
                        replacer_step->find,
                        replacer_step->replace
                        }));
                    break;
            }
        }
        cfg.set_pipeline(steps_str);
    }

    void url_pipeline::reset() {
        steps.clear();
        steps.push_back(make_shared<bt::pipeline::o365>());
        steps.push_back(make_shared<bt::pipeline::unshortener>());
        save();
    }
}