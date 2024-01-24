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

        if(up.url.starts_with(CustomProtoName) && up.url.size() > CustomProtoName.size() + 3) {
            up.url = up.url.substr(CustomProtoName.size() + 3);
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

        steps.push_back(make_shared<bt::pipeline::o365>());

        if(cfg.get_unshort_enabled()) {
            steps.push_back(make_shared<bt::pipeline::unshortener>());
        }

        //auto replacer_rules = cfg.get_pipeline_replacement_rules();
        steps.push_back(make_shared<bt::pipeline::replacer>(""));
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
                        replacer_step->kind == bt::pipeline::replacer_kind::find_replace ? "substring" : "regex",
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