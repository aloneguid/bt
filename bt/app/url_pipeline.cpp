#include "url_pipeline.h"
#include "pipeline/unshortener.h"
#include "pipeline/o365.h"
#include "pipeline/replacer.h"
#include "../globals.h"

using namespace std;

namespace bt {

    url_pipeline::url_pipeline(config& cfg) : cfg{cfg} {
        reconfigure();
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

    void url_pipeline::reconfigure() {
        steps.clear();

        steps.push_back(make_shared<bt::pipeline::o365>());

        if(cfg.get_unshort_enabled()) {
            steps.push_back(make_shared<bt::pipeline::unshortener>());
        }

        auto replacer_rules = cfg.get_pipeline_replacement_rules();
        if(!replacer_rules.empty()) {
            steps.push_back(make_shared<bt::pipeline::replacer>(replacer_rules));
        }
    }
}