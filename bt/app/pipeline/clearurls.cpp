#include "clearurls.h"
#include "clearurls_rules.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <regex>
#include "str.h"

using namespace std;
using namespace grey::common;
using json = nlohmann::json;

// global instance of parsed database
static optional<json> rules_json;

namespace bt::pipeline {
    clearurls::clearurls() : url_pipeline_step(url_pipeline_step_type::clearurls) {
        load_db();
    }

    void clearurls::process(click_payload& cp) {
        cp.url = clean(cp, url{cp.url}).to_string();
    }

    void clearurls::load_db() {
        if(rules_json.has_value()) return;

        rules_json = json::parse(ClearUrlsJson);
    }

    url clearurls::clean(click_payload& cp, url u, int depth) {
        if(depth > 5 || !rules_json.has_value()) return u;

        const auto& providers = rules_json->at("providers");

        for (auto& [name, provider] : providers.items()) {
            std::string url_pattern = provider.value("urlPattern", "");
            if (url_pattern.empty()) continue;

            std::regex provider_regex(url_pattern, std::regex::icase);
            std::string url_str = u.to_string();
            if (!std::regex_search(url_str, provider_regex)) continue;

            // Skip this provider entirely if any exception pattern matches.
            bool excepted = false;
            for (auto& exception_pattern : provider.value("exceptions", nlohmann::json::array())) {
                std::regex exception_regex(exception_pattern.get<std::string>(), std::regex::icase);
                if (std::regex_search(url_str, exception_regex)) {
                    excepted = true;
                    break;
                }
            }
            if (excepted) continue;

            // Redirection unwrapping: if this URL is really a wrapper around
            // another URL, extract the target and recurse on it.
            bool redirected = false;
            for (auto& redir_pattern : provider.value("redirections", nlohmann::json::array())) {
                std::regex redir_regex(redir_pattern.get<std::string>(), std::regex::icase);
                std::smatch match;
                if (std::regex_search(url_str, match, redir_regex) && match.size() > 1) {
                    std::string target = str::url_decode(match[1].str());
                    u = clean(cp, url{target}, depth + 1);
                    redirected = true;
                    break;
                }
            }
            if (redirected) continue;

            // rawRules apply as regex substitutions across the whole URL string.
            for (auto& raw_rule : provider.value("rawRules", nlohmann::json::array())) {
                std::regex raw_regex(raw_rule.get<std::string>(), std::regex::icase);
                url_str = std::regex_replace(url_str, raw_regex, "");
            }
            u = url{url_str};

            // Build the set of query-parameter-name patterns to strip:
            // both plain tracking rules and referral-marketing rules.
            std::vector<std::regex> param_regexes;
            for (auto& rule : provider.value("rules", nlohmann::json::array()))
                param_regexes.emplace_back("^(?:" + rule.get<std::string>() + ")$", std::regex::icase);
            for (auto& rule : provider.value("referralMarketing", nlohmann::json::array()))
                param_regexes.emplace_back("^(?:" + rule.get<std::string>() + ")$", std::regex::icase);

            auto params = u.parameters;
            std::vector<std::pair<std::string, std::string>> cleaned_params;
            cleaned_params.reserve(params.size());
            for (auto& [key, value] : params) {
                bool is_tracker = false;
                for (auto& re : param_regexes) {
                    if (std::regex_match(key, re)) {
                        is_tracker = true;
                        cp.trackers_removed += 1;
                        break;
                    }
                }
                if (!is_tracker) cleaned_params.emplace_back(key, value);
            }
            u.parameters = cleaned_params;
        }

        return u;
    }
}
