#pragma once

#include <string>

namespace bt {
    enum class match_scope {
        any     = 0,
        domain  = 1,
        path    = 2
    };

    class match_rule {
    public:
        explicit match_rule(const std::string& line);

        bool is_match(const std::string& line) const;

        std::string value;
        match_scope scope{match_scope::any};
        int priority{0};
        bool is_regex{false};
        bool app_mode{false};
        std::string firefox_container;

        bool operator==(const match_rule& other) const;

        std::string to_string() const;
        std::string to_line() const;

        static std::string to_string(match_scope s);
        static match_scope to_match_scope(const std::string& s);

        static bool parse_url(const std::string& url, std::string& proto, std::string& host, std::string& path);

    private:
        bool contains(const std::string& input, const std::string& value) const;
    };
}