#include "url_pipeline.h"
#include "pipeline/unshortener.h"
#include "pipeline/o365.h"
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

        if(cfg.pipeline_unwrap_o365) {
            steps.push_back(make_shared<bt::pipeline::o365>());
        }

        if(cfg.pipeline_unshorten) {
            steps.push_back(make_shared<bt::pipeline::unshortener>());
        }

        for(string s : cfg.pipeline_substitutions) {
            steps.push_back(make_shared<bt::pipeline::replacer>(s));
        }
    }

    std::shared_ptr<bt::pipeline::replacer> url_pipeline::get_replacer_step(size_t idx) {
        // enumerate all steps until we find the indexed replacer step
        size_t i = 0;
        for(auto step : steps) {
            if(step->type == url_pipeline_step_type::find_replace) {
                if(i == idx) {
                    return std::static_pointer_cast<bt::pipeline::replacer>(step);
                }
                i++;
            }
        }
        return std::shared_ptr<bt::pipeline::replacer>();   // empty pointer
    }
}