#pragma once
#include <string>

namespace bt::security {
    class clearurl_provider {
    public:
        clearurl_provider(const std::string& regex)
    };

    /**
     * @brief ClearURLs (https://gitlab.com/ClearURLs/ClearUrls) re-implementation in C++.
     * To refresh rules from the online source, run prebuild.py.
     * 
     * How the format works:
     * 1. match rule by "urlPattern" and skip if not matched.
     * 2. if matched, try to match any "exceptions" and skip if any matched.
     * 3. if "redirections" are defined todo https://github.com/walterl/uroute/blob/d6b26a0d2fdcabb3ed4ae41c0376bb833b696be6/uroute/url.py#L87
     * 4. todo
    */
    class clearurls {
    public:

    };
}