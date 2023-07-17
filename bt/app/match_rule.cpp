#include "match_rule.h"
#include <str.h>
#include <fmt/core.h>
#include <regex>

using namespace std;

namespace bt {
    match_rule::match_rule(const std::string& line) {
        string src = line;
        str::trim(src);

        for(string p : str::split(src, "|")) {
            if(p.empty()) continue;

            size_t idx = p.find_first_of(':');
            if(idx != string::npos) {
                string k = p.substr(0, idx);
                string v = (idx + 1 < p.size())
                    ? p.substr(idx + 1)
                    : "";

                if(k == "scope") {
                    scope = to_match_scope(v);
                } else if(k == "priority") {
                    priority = str::to_int(v);
                } else if(k == "mode") {
                    if(v == "app") app_mode = true;
                } else if(k == "type") {
                    if(v == "regex") is_regex = true;
                }

            } else {
                value = p;
            }
        }
    }

    std::string match_rule::to_string() const {
        switch(scope) {
            case match_scope::any:
                return value;
            case match_scope::domain:
                return fmt::format("{} (in domain)", value);
            case match_scope::path:
                return fmt::format("{} (in path)", value);
        }

        return "?";
    }

    std::string match_rule::to_line() const {
        string s = value;
        str::trim(s);
        if(s.empty()) return "";

        vector<string> parts;

        if(scope != match_scope::any) {
            parts.push_back(fmt::format("scope:{}", to_string(scope)));
        }

        if(priority > 0) {
            parts.push_back(fmt::format("priority:{}", priority));
        }

        if(app_mode) {
            parts.push_back("mode:app");
        }

        if(is_regex) {
            parts.push_back("type:regex");
        }

        parts.push_back(s);

        return str::join(parts.begin(), parts.end(), "|");
    }

    std::string match_rule::to_string(match_scope s) {
        switch(s) {
            case bt::match_scope::any:
                return "any";
            case bt::match_scope::domain:
                return "domain";
            case bt::match_scope::path:
                return "path";
            default:
                return "unknown";
        }
    }

    match_scope match_rule::to_match_scope(const std::string& s) {
        if(s == "domain") return match_scope::domain;
        if(s == "path") return match_scope::path;
        return match_scope::any;
    }

    bool match_rule::parse_url(const string& url, string& proto, string& host, string& path) {
        const string prot_end("://");
        proto = host = path = "";

        size_t idx = url.find(prot_end);
        if(idx == string::npos) {
            proto = "";
            host = url;
        } else {
            proto = url.substr(0, idx);
            host = idx + prot_end.size() < url.size()
                ? url.substr(idx + prot_end.size())
                : "";
        }

        idx = host.find_first_of('/');
        if(idx == string::npos) {
            path = "";
        } else {

            path = host.substr(idx + 1);
            host = host.substr(0, idx);
        }

        return true;
    }

    bool match_rule::contains(const string& input, const string& value) const {
        if(is_regex) {
            regex r{value, regex_constants::icase};
            return regex_match(input, r);
        } else {
            return str::contains_ic(input, value);
        }
    }

    bool match_rule::is_match(const std::string& line) const {

        if(value.empty()) return false;

        string src = line;
        str::trim(src);

        if(line.empty()) return false;

        switch(scope) {
            case match_scope::any: {
                return contains(src, value);
            }
            case match_scope::domain: {
                string proto, host, path;
                if(!parse_url(src, proto, host, path)) return false;
                return contains(host, value);
            }
            case match_scope::path: {
                string proto, host, path, query;
                if(!parse_url(src, proto, host, path)) return false;
                return contains(path, value);
            }
        }

        return false;
    }
}