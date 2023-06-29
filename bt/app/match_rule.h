#pragma once

#include <string>

namespace bt {
    enum class match_scope {
        any = 0,
        domain = 1,
        path = 2
    };

    class match_rule {
    public:
        match_rule() {}
        match_rule(const std::string& line);

        bool is_match(const std::string& line) const;

        std::string value;
        match_scope scope{match_scope::any};

        bool operator==(const match_rule& other) const;

        std::string to_string() const;
        std::string to_line() const;

        static bool parse_url(const std::string& url, std::string& proto, std::string& host, std::string& path);
    };
}