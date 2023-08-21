#include "clearurls.h"
#include <regex>
#include "../config.h"
#include <filesystem>
#include "win32/http.h"
#include "fss.h"
#include <nlohmann/json.hpp>
#include "url.h"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

namespace bt::security {

    const string DataFileName = "clearurls_db.json";

    clearurl_provider::clearurl_provider(const std::string& name, const std::string& regex,
        std::vector<std::string> rules,
        std::vector<std::string> exceptions,
        std::vector<std::string> redirections,
        std::vector<std::string> raw_rules)
        : name{name}, regex{regex}, rules{rules}, exceptions{exceptions}, redirections{redirections}, raw_rules{raw_rules} {
    }

    clearurls::clearurls() {
        load_db();
    }

    clearurl_result clearurls::clear(const std::string& abs_url) {

        bool is_dirty{false};
        vector<string> dirty_parameters;
        string clean_url = abs_url;

        // find a match
        for(const clearurl_provider& p : providers) {
            regex p_rgx{p.regex, regex_constants::icase};
            if(regex_search(abs_url, p_rgx)) {
                // check for exceptions
                for(const string& x : p.exceptions) {
                    regex x_rgx{x, regex_constants::icase};
                    if(regex_search(abs_url, x_rgx)) {
                        continue;
                    }
                }

                // match found!

                // we are going to try "rawRules" (bold replacement)
                for(const string& rr : p.raw_rules) {
                    regex rr_rgx{rr, regex_constants::icase};
                    smatch sm;

                    if(regex_search(clean_url, sm, rr_rgx)) {
                        clean_url.replace(sm.position(), sm.length(), "");
                        is_dirty = true;
                    }
                }

                url u{clean_url};
                for(const auto& kv : u.parameters) {
                    string pn = kv.first;

                    // match parameter name across all the rules
                    for(const string& rule : p.rules) {
                        regex r_rgx{rule, regex_constants::icase};
                        if(regex_match(pn, r_rgx)) {
                            dirty_parameters.push_back(pn);
                        }
                    }
                }
                if(!dirty_parameters.empty()) {
                    for(const string& pn : dirty_parameters) {
                        for(int i = u.parameters.size() - 1; i >= 0; i--) {
                            if(u.parameters[i].first == pn) {
                                u.parameters.erase(u.parameters.begin() + i);
                                break;
                            }
                        }
                    }
                    clean_url = u.to_string();
                    is_dirty = true;
                }

                break;
            }

        }

        return clearurl_result{
            abs_url,
            clean_url,
            is_dirty,
            dirty_parameters};
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
                        j_strarr(j_p, "redirections"),
                        j_strarr(j_p, "rawRules"));
                }
            }


        } catch(json::type_error) {
        }
        
    }
}