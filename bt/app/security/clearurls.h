#pragma once
#include <string>
#include <vector>

namespace bt::security {
    class clearurl_provider {
    public:
        clearurl_provider(const std::string& name, const std::string& regex,
            std::vector<std::string> rules,
            std::vector<std::string> exceptions,
            std::vector<std::string> redirections,
            std::vector<std::string> raw_rules);

        std::string name;
        std::string regex;
        std::vector<std::string> rules;
        std::vector<std::string> exceptions;
        std::vector<std::string> redirections;
        std::vector<std::string> raw_rules;
    };

    class clearurl_result {
    public:
        std::string original_url;
        std::string clear_url;
        bool is_dirty;
        std::vector<std::string> dirty_parameters;
    };

    /**
     * @brief ClearURLs (https://gitlab.com/ClearURLs/ClearUrls) re-implementation in C++.
     * To refresh rules from the online source, run prebuild.py.
     * 
     * Author's python reference implementation: https://gitlab.com/ClearURLs/ClearUrls/-/snippets/1834899
     * 
     * How the format works:
     * 1. match rule by "urlPattern" and skip if not matched.
     * 2. if matched, try to match any "exceptions" and skip if any matched.
     * 3. if "redirections" are defined, match them. If a redirection is matched, the url has to be transformed to search results's first subgroup (i.e. https://google.com/....&url=(submatch)&.... redirects to "submatch"
     * 4. perform parameter replacements from the "rules" section. "rawRules" perform replacement on the entire URL.
    */
    class clearurls {
    public:
        clearurls();

        clearurl_result clear(const std::string& url);

    private:
        std::vector<clearurl_provider> providers;
        void load_db();
    };
}