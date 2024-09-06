#include "url_pipeline.h"
#include "pipeline/unshortener.h"
#include "pipeline/o365.h"
#include "pipeline/script.h"
#include "../globals.h"

using namespace std;

namespace bt {

    url_pipeline::url_pipeline(config& cfg) : cfg{cfg} {
        load();
    }

    void url_pipeline::clean(std::string& s) {
        // remove custom protocol prefix
        size_t sz = string(CustomProtoName).size();
        if(s.starts_with(CustomProtoName) && s.size() > sz + 3) {
            s = s.substr(sz + 3);
        }

        // Firefox for some reason removes ':' when opening custom protocol links, so we need to add it back
        size_t idx = s.find("://");
        if(idx == string::npos) {
            idx = s.find("//");
            if(idx != string::npos) {
                s = s.substr(0, idx) + ":" + s.substr(idx);
            }
        }
    }

    void url_pipeline::process(click_payload& up) {
        clean(up.url);

        for(auto& step : steps) {
            step->process(up);
        }
    }

    std::vector<url_pipeline_processing_step> url_pipeline::process_debug(click_payload& cp) {
        
        clean(cp.url);

        vector<url_pipeline_processing_step> r;

        for(auto& step : steps) {
            click_payload before = cp;
            step->process(cp);
            r.push_back({step, before, cp});
        }

        return r;
    }

    void url_pipeline::load() {
        steps.clear();

        if(cfg.pipeline_unwrap_o365) {
            steps.push_back(make_shared<bt::pipeline::o365>());
        }

        if(cfg.pipeline_unshorten) {
            steps.push_back(make_shared<bt::pipeline::unshortener>());
        }

        if(cfg.pipeline_substitute) {
            for(string s : cfg.pipeline_substitutions) {
                steps.push_back(make_shared<bt::pipeline::replacer>(s));
            }
        }

        if(cfg.pipeline_script) {
            for(string fn : g_script.ppl_function_names) {
                steps.push_back(make_shared<bt::pipeline::script>(fn));
            }
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