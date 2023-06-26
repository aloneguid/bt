#include "browser.h"
#include <windows.h>
#include <filesystem>
#include <set>
#include <algorithm>
#include <cctype>
#include <functional>
#include "win32/shell.h"
#include "win32/process.h"
#include "str.h"
#include "../globals.h"
#include <fmt/core.h>
#include "config.h"

namespace fs = std::filesystem;
using namespace std;

namespace bt {
    //const string browser_instance::URL_ARG_NAME{ L"%url%" };

    const string lad = win32::shell::get_local_app_data_path();
    vector<shared_ptr<browser>> browser::cache;

    /// Try to find in the Haystack the Needle - ignore case
    bool contains_ic(const std::string& haystack, const std::string& needle) {
        auto it = std::search(
            haystack.begin(), haystack.end(),
            needle.begin(), needle.end(),
            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        );
        return (it != haystack.end());
    }

    match_rule::match_rule(const std::string& line) {
        string src = line;
        str::trim(src);

        for(string p : str::split(src, "|")) {
            if(p.empty()) continue;

            size_t idx = p.find_first_of(':');
            if(idx != string::npos) {
                string key = p.substr(0, idx);
                if(key == "scope" && (idx + 1 < p.size())) {
                    scope = (match_scope)str::to_int(p.substr(idx + 1));
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
        if(!s.empty()) {
            if(scope != match_scope::any) {
                s = fmt::format("|scope:{}|{}", (int)scope, s);
            }
        }
        return s;
    }

    bool match_rule::parse_url(const string& url, string& proto, string& host, string& path) {
        const string prot_end("://");

        size_t idx = url.find_first_of(prot_end);
        if(idx == string::npos) {
            proto = "";
            host = url;
        } else {
            proto = url.substr(0, idx);
            host = idx+ prot_end.size() < url.size() 
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

        switch(scope) {
            case match_scope::any:
                return contains_ic(line, value);
            case match_scope::domain: {
                string proto, host, path;
                if(!parse_url(line, proto, host, path)) return false;
                return contains_ic(host, value);
            }
            case match_scope::path: {
                string proto, host, path, query;
                if(!parse_url(line, proto, host, path)) return false;
                return contains_ic(path, value);
            }
        }

        return false;        
    }

    browser::browser(
        const std::string& id,
        const std::string& name,
        const std::string& open_cmd,
        bool is_system)
        : id{id}, name{ name }, open_cmd{ open_cmd },
        is_chromium{ false }, is_firefox{ false },
        is_system{ is_system }
        //pc_cpu{win32::perf_counter::make_process_processor_time(get_image_name(open_cmd))}
    {
        str::trim(this->name);

        str::trim(this->open_cmd, "\"");

        if (this->id == "msedge") {
            is_chromium = true;
            vdf = "Microsoft\\Edge\\User Data";
        } else if (this->id == "chrome") {
            is_chromium = true;
            vdf = "Google\\Chrome\\User Data";
        } else if(this->id == "vivaldi") {
            is_chromium = true;
            vdf = "Vivaldi\\User Data";
        } else if(this->id == "brave") {
            is_chromium = true;
            vdf = "BraveSoftware\\Brave-Browser\\User Data";
        } else if (this->id == "firefox") {
            is_firefox = true;
            vdf = "Mozilla\\Firefox\\Profiles";
        }
    }

    bool match_rule::operator==(const match_rule& other) const {
        return value == other.value && scope == other.scope;
    }

    bool operator==(const browser& b1, const browser& b2) {
        return b1.id == b2.id;
    }

    std::vector<win32::process> browser::get_system_processes() {
        vector<win32::process> r;
        for(auto& p : win32::process::enumerate()) {
            if(p.get_module_filename() != open_cmd) continue;
            r.push_back(p);
        }
        return r;
    }

    bool browser::is_running() {
        auto all_procs = win32::process::enumerate();
        return std::any_of(all_procs.begin(), all_procs.end(),
            [this](const win32::process& p) { return p.get_module_filename() == open_cmd; });
    }

    std::vector<std::shared_ptr<browser>> browser::get_cache(bool reload) {
        if(reload || cache.empty())
            cache = config::i.load_browsers();

        return cache;
    }

    void browser::persist_cache() {
        config::i.save_browsers(cache);
    }

    std::vector<std::shared_ptr<browser_instance>> browser::to_instances(const std::vector<std::shared_ptr<browser>> browsers) {
        vector<shared_ptr<browser_instance>> r;
        for(const auto& b : browsers) {
            for(const auto& bi : b->instances) {
                r.push_back(bi);
            }
        }
        return r;

    }

    std::shared_ptr<browser_instance> browser::find_profile_by_long_id(const vector<shared_ptr<browser>> browsers,
        const std::string& long_id, bool& found) {
        found = false;

        // try to find
        for (auto b : browsers) {
            for (auto& p : b->instances) {
                if (p->long_id() == long_id) {
                    found = true;
                    return p;
                }
            }
        }

        // return default
        return browsers[0]->instances[0];
    }

    std::vector<browser_match_result> browser::match(
        const std::vector<shared_ptr<browser>> browsers,
        const std::string& raw_url, std::string& url_to_open) {
        vector<browser_match_result> r;
        string url = raw_url;
        preprocess_url(url);
        url_to_open = url;

        // which browser should we use?
        for (auto b : browsers) {
            for (auto i : b->instances) {
                match_rule mr{ "" };
                if (i->is_match(url, mr)) {
                    r.emplace_back(i, mr);
                }
            }
        }

        if (r.empty()) {
            r.emplace_back(get_fallback(browsers), match_rule{ "fallback browser" });
        }

        return r;
    }

    shared_ptr<browser_instance> browser::get_fallback(const std::vector<shared_ptr<browser>> browsers) {
        string lsn = config::i.get_fallback_long_sys_name();

        bool found;
        auto bi = find_profile_by_long_id(browsers, lsn, found);

        return found ? bi : browsers[0]->instances[0];
    }

    std::vector<std::shared_ptr<browser>> browser::merge(
        std::vector<std::shared_ptr<browser>> new_set, std::vector<std::shared_ptr<browser>> old_set) {
        
        // todo: this would be nice to rewrite in modern functional C++
        vector<shared_ptr<browser>> r;

        for (shared_ptr<browser> b_new : new_set) {
            // find corresponding browser
            auto b_old_it = std::find_if(
                old_set.begin(), old_set.end(),
                [b_new](shared_ptr<browser> el) { return el->id == b_new->id; });

            if (b_old_it != old_set.end()) {
                shared_ptr<browser> b_old = *b_old_it;

                // profiles

                // merge old data into new profiles
                for (shared_ptr<browser_instance> bi_new : b_new->instances) {
                    auto bi_old_it = std::find_if(
                        b_old->instances.begin(), b_old->instances.end(),
                        [bi_new](shared_ptr<browser_instance> el) {return el->id == bi_new->id; });

                    if (bi_old_it == b_old->instances.end()) continue;
                    shared_ptr<browser_instance> bi_old = *bi_old_it;

                    // merge rules
                    for (auto& rule : bi_old->rules) {
                        bi_new->add_rule(rule->value);
                    }
                }
            }

            r.push_back(b_new);
        }

        // add user browsers from the old set
        for(shared_ptr<browser> b_custom : old_set) {
            if(b_custom->is_system) continue;

            r.push_back(b_custom);
        }

        return r;
    }

    void browser::preprocess_url(std::string& url) {
        // https://eur02.safelinks.protection.outlook.com/?url=https%3A%2F%2Fdocs.microsoft.com%2Fen-us%2Fazure%2Fdatabricks%2Fsecurity%2Fsecrets%2Fsecrets&data=04%7C01%7Civan.gavryliuk%40digital.homeoffice.gov.uk%7C69bc2c30202c4b2d732708d94c58d32a%7C90a6982e16354aa087a6d5d63c7bc66e%7C0%7C0%7C637624766652025108%7CUnknown%7CTWFpbGZsb3d8eyJWIjoiMC4wLjAwMDAiLCJQIjoiV2luMzIiLCJBTiI6Ik1haWwiLCJXVCI6Mn0%3D%7C1000&sdata=xRSD%2BBHpkwBES5SXKXSxQOCASAsRYX839AV37tezvtU%3D&reserved=0

        /*if(url.starts_with("https://urldefense.com/v3/__")) {
            size_t first = url.find_first_of("__");
            size_t last = url.find_last_of("__");
            if(first != string::npos && last != string::npos) {
                url = url.substr(first + 2, last - first - 2);
            }
        }*/
    }

    std::string browser::get_image_name(const std::string& open_cmd) {
        if(open_cmd.empty()) return open_cmd;
        return fs::path{open_cmd}.filename().replace_extension().string();
    }

    browser_instance::browser_instance(
        shared_ptr<browser> b,
        const std::string& id,
        const std::string& name,
        const std::string& launch_arg,
        const std::string& icon_path,
        const std::string& open_cmd)
        : b{ b },

        id{ id },
        name{ name },

        launch_arg{ launch_arg },
        icon_path{ icon_path } {

        this->open_cmd = open_cmd.empty() ? b->open_cmd : open_cmd;
    }

    browser_instance::~browser_instance() {}

    void browser_instance::launch(url_payload up) const {
        string final_url = up.url;
        bool is_xbt;
        if (final_url.starts_with(CustomProtoName) && final_url.size() > CustomProtoName.size() + 3) {
            final_url = final_url.substr(CustomProtoName.size() + 3);
            is_xbt = true;
        } else {
            is_xbt = false;
        }

        string arg = launch_arg;

        if(arg.empty()) {
            arg = final_url;
        } else {
            size_t pos = arg.find(URL_ARG_NAME);
            if(pos != string::npos) {
                arg.replace(pos, URL_ARG_NAME.size(), final_url);
            }
        }

        win32::shell::exec(open_cmd, arg);
    }

    bool browser_instance::is_match(const std::string& url, match_rule& mr) const {
        for (const auto& rule : rules) {
            if (rule->is_match(url)) {
                mr = *rule;
                return true;
            }
        }

        return false;
    }

    bool browser_instance::add_rule(const std::string& rule_text) {
        auto new_rule = make_shared<match_rule>(rule_text);

        for (const auto& rule : rules) {
            if (*rule == *new_rule)
                return false;
        }

        rules.push_back(new_rule);

        return true;
    }

    void browser_instance::delete_rule(const std::string& rule_text) {
        std::erase_if(rules, [rule_text](auto r) { return r->value == rule_text; });
    }

    std::string browser_instance::get_best_icon_path() const {
        string r = icon_path;
        if (r.empty()) r = open_cmd;
        if (r.empty()) r = b->open_cmd;
        return r;
    }

    std::string browser_instance::get_best_browser_icon_path() const {
        return open_cmd.empty() ? icon_path : open_cmd;
    }

    bool browser_instance::is_singular() const {
        return count_if(b->instances.begin(), b->instances.end(), [](auto i) {return !i->is_incognito; }) == 1;
    }

    std::string browser_instance::get_best_display_name() const {
        if(is_incognito) return fmt::format("Private {}", b->name);

        if(b->is_system && is_singular()) return b->name;

        return name;
    }

    vector<string> browser_instance::get_rules_as_text_clean() const {
        vector<string> res;
        for (const auto& r : rules) {
            string s = r->to_line();
            if(!s.empty())
                res.push_back(s);
        }
        return res;
    }

    void browser_instance::set_rules_from_text(std::vector<std::string> rules_txt) {
        for (string& rule : rules_txt) {
            string clean_rule = rule;
            str::trim(clean_rule);
            if(!clean_rule.empty()) {
                add_rule(rule);
            }
        }
    }
}