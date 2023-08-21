#include "clearurls.h"
#include <regex>
#include "../config.h"
#include <filesystem>
#include "win32/http.h"
#include "fss.h"
#include <nlohmann/json.hpp>

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

namespace bt::security {

    const string DataFileName = "clearurls_db.json";

    clearurl_provider::clearurl_provider(const std::string& name, const std::string& regex,
        std::vector<std::string> qps,
        std::vector<std::string> exceptions,
        std::vector<std::string> redirections) : name{name}, regex{regex}, qps{qps}, exceptions{exceptions}, redirections{redirections} {
    }

    clearurls::clearurls() {
        load_db();
    }

    bool clearurls::clear(const std::string& url) {
        // find a match
        for(const clearurl_provider& p : providers) {
            regex p_rgx{p.regex, regex_constants::icase};
            if(regex_search(url, p_rgx)) {
                // check for exceptions
                for(const string& x : p.exceptions) {
                    regex x_rgx{x, regex_constants::icase};
                    if(regex_search(url, x_rgx)) {
                        continue;
                    }
                }

                // match found!

            }

            return false;

        }
    }

    vector<string> j_strarr(json& j, string prop_name) {
        vector<string> r;
        auto j_coll = j[prop_name];
        for(auto j_el : j_coll) {
            r.push_back(j_el.get<string>());
        }
        return r;
    }

    void clearurls::load_db() {
        string data_path = config::get_data_file_path(DataFileName);
        
        if(!fs::exists(data_path)) {
            // download latest

            win32::http h;
            string txt = h.get("https://gitlab.com/ClearURLs/rules/-/raw/master/data.min.json");
            if(!txt.empty()) {
                fss::write_file_as_string(data_path, txt);
            }
        }

        // parse db
        providers.clear();
        string txt = fss::read_file_as_string(data_path);

        try {
            auto j = json::parse(txt);

            auto j_providers = j["providers"];
            if(j_providers.is_object()) {
                for(json::iterator it = j_providers.begin(); it != j_providers.end(); ++it) {
                    string provider_name = it.key();

                    auto j_p = it.value();
                    string regex = j_p["urlPattern"].get<string>();

                    providers.emplace_back(provider_name, regex,
                        j_strarr(j_p, "rules"),
                        j_strarr(j_p, "exceptions"),
                        j_strarr(j_p, "redirections"));
                }
            }


        } catch(json::type_error) {
        }
        
    }
}