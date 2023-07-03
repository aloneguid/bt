#include "match_rule.h"
#include <str.h>
#include <fmt/core.h>

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

        if(scope != match_scope::any) {
            s = fmt::format("scope:{}|{}", to_string(scope), s);
        }

        if(priority > 0) {
            s = fmt::format("priority:{}|{}", priority, s);
        }

        return s;
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

        size_t idx = url.find_first_of(prot_end);
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

    bool match_rule::is_match(const std::string& line) const {

        if(value.empty()) return false;

        string src = line;
        str::trim(src);

        if(line.empty()) return false;

        switch(scope) {
            case match_scope::any:
                return str::contains_ic(src, value);
            case match_scope::domain: {
                string proto, host, path;
                if(!parse_url(src, proto, host, path)) return false;
                return str::contains_ic(host, value);
            }
            case match_scope::path: {
                string proto, host, path, query;
                if(!parse_url(src, proto, host, path)) return false;
                return str::contains_ic(path, value);
            }
        }

        return false;
    }
}