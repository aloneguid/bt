#pragma once
#include <string>
#include <vector>

namespace bt::security {
    class clearurl_provider {
    public:
        clearurl_provider(const std::string& name, const std::string& regex,
            std::vector<std::string> qps,
            std::vector<std::string> exceptions,
            std::vector<std::string> redirections);

        std::string name;
        std::string regex;
        std::vector<std::string> qps;
        std::vector<std::string> exceptions;
        std::vector<std::string> redirections;
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
     * 4. perform parameter replacements
    */
    class clearurls {
    public:
        clearurls();

        bool clear(const std::string& url);

    private:
        std::vector<clearurl_provider> providers;
        void load_db();
    };
}