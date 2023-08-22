#include "url_pipeline.h"
#include "pipeline/unshortener.h"

using namespace std;

namespace bt {

    //const string ClearUrlsDataFileName = "clearurls_db.json";

    url_pipeline::url_pipeline(config& cfg) : cfg{cfg} {
        reconfigure();
    }

    std::string url_pipeline::process(const std::string& url) {
        string r = url;
        for(auto& step : steps) {
            r = step->process(r);
        }
        return r;
    }

    void url_pipeline::reconfigure() {
        steps.clear();

        //if(g_config.get_clearurls_enabled()) {
        //    string data_path = config::get_data_file_path(ClearUrlsDataFileName);

        //    steps.push_back(make_unique<bt::security::clearurls>());
        //}

        if(cfg.get_unshort_enabled()) {
            steps.push_back(make_unique<bt::pipeline::unshortener>());
        }
    }
}