#pragma once

#include <string>
#include "script_site.h"
#include "click_payload.h"

namespace bt {
    enum class match_scope : size_t {
        any     = 0,
        domain  = 1,
        path    = 2
    };

    enum class match_location : size_t {
        url             = 0,
        window_title    = 1,
        process_name    = 2,
        lua_script      = 3
    };

    class match_rule {
    public:
        explicit match_rule(const std::string& line);

        bool is_match(const click_payload& up, const script_site& script) const;
        bool is_match(const click_payload& up) const;
        bool is_match(const std::string& url) const;

        std::string value;
        match_location loc{match_location::url};
        match_scope scope{match_scope::any};
        int priority{0};
        bool is_regex{false};
        bool app_mode{false};
        bool is_fallback{false};

        /**
         * @brief If this rule has extra actions configured (such as opening in a container) you should apply them to the payload.
         * @param up payload to modify
         */
        void apply_to(click_payload& up) const;

        // UI helpers
        bool ui_test_url_matches;

        bool operator==(const match_rule& other) const;

        std::string to_string(bool include_type = true) const;
        std::string to_line() const;
        std::string get_type_string() const;

        static std::string to_string(match_scope s);
        static std::string to_string(match_location s);
        static match_scope to_match_scope(const std::string& s);
        static match_location to_match_location(const std::string& s);

        static bool parse_url(const std::string& url, std::string& proto, std::string& host, std::string& path);

    private:
        bool contains(const std::string& input, const std::string& value) const;
    };
}