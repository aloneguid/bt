#include "match_rule.h"
#include <str.h>
#include <fmt/core.h>
#include <regex>

using namespace std;

namespace bt {

    const string ScopeKey = "scope";
    const string LocationKey = "loc";
    const string PriorityKey = "priority";
    const string ModeKey = "mode";
    const string AppKey = "app";
    const string TypeKey = "type";

    match_rule::match_rule(const std::string& line) {
        string src = line;
        str::trim(src);
        auto parts = str::split_pipe(src);

        for(string& p : parts) {
            if(p.empty()) continue;

            size_t idx = p.find_first_of(':');
            if(idx != string::npos) {
                string k = p.substr(0, idx);
                string v = (idx + 1 < p.size())
                    ? p.substr(idx + 1)
                    : "";

                if(k == ScopeKey) {
                    scope = to_match_scope(v);
                } else if(k == LocationKey) {
                    loc = to_match_location(v);
                } else if(k == PriorityKey) {
                    priority = str::to_int(v);
                } else if(k == ModeKey) {
                    if(v == AppKey) app_mode = true;
                } else if(k == TypeKey) {
                    if(v == "regex") is_regex = true;
                }

            } else {
                value = p;
            }
        }
    }

    std::string match_rule::to_string() const {

        if(is_fallback) return value;

        string r = fmt::format("{} '{}' in ", 
            is_regex ? "regular expression" : "substring",
            value);

        switch(loc) {
            case match_location::url:
            {
                switch(scope) {
                    case match_scope::any:
                        r += "URL";
                        break;
                    case match_scope::domain:
                        r += fmt::format("domain part of the URL", value);
                        break;
                    case match_scope::path:
                        r += fmt::format("query part of the URL", value);
                        break;
                }
            }
            break;
            case match_location::window_title:
                r += "application title";
                break;
            case match_location::process_name:
                r += "process name";
                break;
        }

        return r;
    }

    std::string match_rule::to_line() const {
        string s = value;
        str::trim(s);
        if(s.empty()) return "";

        vector<string> parts;

        if(loc != match_location::url) {
            parts.push_back(fmt::format("{}:{}", LocationKey, to_string(loc)));
        }

        if(scope != match_scope::any) {
            parts.push_back(fmt::format("{}:{}", ScopeKey, to_string(scope)));
        }

        if(priority > 0) {
            parts.push_back(fmt::format("{}:{}", PriorityKey, priority));
        }

        if(app_mode) {
            parts.push_back(fmt::format("{}:app", ModeKey));
        }

        if(is_regex) {
            parts.push_back(fmt::format("{}:regex", TypeKey));
        }

        parts.push_back(s);

        return str::join_with_pipe(parts);
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

    std::string match_rule::to_string(match_location s) {
        switch(s) {
            case bt::match_location::url:
                return "url";
            case bt::match_location::window_title:
                return "window_title";
            case bt::match_location::process_name:
                return "process_name";
            default:
                return "";
        }
    }

    match_scope match_rule::to_match_scope(const std::string& s) {
        if(s == "domain") return match_scope::domain;
        if(s == "path") return match_scope::path;
        return match_scope::any;
    }

    match_location match_rule::to_match_location(const std::string& s) {
        if(s == "window_title") return match_location::window_title;
        if(s == "process_name") return match_location::process_name;
        return match_location::url;
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
            try {
                regex r{value, regex_constants::icase};
                return regex_match(input, r);
            } catch(const std::regex_error& e) {
                // most probably invalid regex pattern
                return false;
            }
        } else {
            return str::contains_ic(input, value);
        }
    }

    bool match_rule::is_match(const url_payload& up) const {

        if(value.empty()) return false;

        string src;
        switch(loc) {
            case match_location::url:
                src = up.match_url;
                break;
            case match_location::window_title:
                src = up.window_title;
                break;
            case match_location::process_name:
                src = up.process_name;
                break;
        }
        str::trim(src);

        if(src.empty()) return false;

        switch(loc) {
            case bt::match_location::url: {
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
            }
            break;
            case bt::match_location::window_title:
                return contains(src, value);
            case bt::match_location::process_name:
                return contains(src, value);
        }

        return false;
    }

    bool match_rule::is_match(const string& url) const {
        url_payload up{url};
        up.match_url = url;
        return is_match(up);
    }
}