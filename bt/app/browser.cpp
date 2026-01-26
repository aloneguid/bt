#include "browser.h"
#include "match_rule.h"
#include <filesystem>
#include <algorithm>
#include "win32/shell.h"
#include "win32/kernel.h"
#include "win32/uwp.h"
#include "win32/user.h"
#include "str.h"
#include <fmt/core.h>

namespace fs = std::filesystem;
using namespace std;

namespace bt {
    const string lad = win32::shell::get_local_app_data_path();
    const string browser::UwpCmdPrefix = "msstore:";

    browser::browser(
        const std::string& id,
        const std::string& name,
        const std::string& open_cmd)
        : id{id}, name{ name }, open_cmd{ open_cmd }
    {
        str::trim(this->name);
    }

    bool operator==(const browser& b1, const browser& b2) {
        return b1.id == b2.id;
    }

    size_t browser::get_total_rule_count() const {
        size_t r{0};
        for(auto i : instances) {
            r += i->rules.size();
        }
        return r;
    }

    std::string browser::get_best_icon_path() const {

        if(!icon_path.empty()) return icon_path;

        if(!is_autodiscovered) {
            if(!instances.empty()) {
                return instances[0]->get_best_icon_path();
            }
        }

        return open_cmd;
    }

    bool browser::contains_profile_id(const std::string& long_id) const {
        for(const auto i : instances) {
            if(i->long_id() == long_id) return true;
        }
        return false;
    }

    std::vector<std::shared_ptr<browser_instance>> browser::to_instances(
        const std::vector<std::shared_ptr<browser>>& browsers,
        bool skip_hidden) {
        vector<shared_ptr<browser_instance>> r;
        for(const auto b : browsers) {
            if(skip_hidden && b->is_hidden) continue;
            for(const auto& bi : b->instances) {
                if(skip_hidden && bi->is_hidden) continue;
                r.push_back(bi);
            }
        }
        return r;

    }

    std::shared_ptr<browser_instance> browser::find_profile_by_long_id(const vector<shared_ptr<browser>>& browsers,
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
        const std::vector<shared_ptr<browser>>& browsers,
        const click_payload& up,
        const string& default_profile_long_id,
        const script_site& script) {
        vector<browser_match_result> r;

        // which browser should we use?
        for (auto b : browsers) {
            for (auto i : b->instances) {
                match_rule mr{ "" };
                if (i->is_match(up, script, mr)) {
                    r.emplace_back(i, mr);
                }
            }
        }

        if (r.empty() && !browsers.empty()) {
            match_rule fbr{"default"};
            fbr.is_fallback = true;
            r.emplace_back(get_default(browsers, default_profile_long_id), fbr);
        }

        // sort by priority, descending
        if(r.size() > 1) {
            std::sort(r.begin(), r.end(), [](const browser_match_result& a, const browser_match_result& b) {
                return a.rule.priority > b.rule.priority;
            });
        }

        return r;
    }

    shared_ptr<browser_instance> browser::get_default(
        const std::vector<shared_ptr<browser>>& browsers,
        const string& default_profile_long_id) {
        bool found;
        auto bi = find_profile_by_long_id(browsers, default_profile_long_id, found);

        return found ? bi : browsers[0]->instances[0];
    }

    std::vector<std::shared_ptr<browser>> browser::merge(
        std::vector<std::shared_ptr<browser>> new_set, std::vector<std::shared_ptr<browser>> old_set) {
        
        // todo: this would be nice to rewrite in modern functional C++
        vector<shared_ptr<browser>> r;

        for (shared_ptr<browser> b_new : new_set) {
            // find corresponding browser by open_cmd
            auto b_old_it = std::find_if(
                old_set.begin(), old_set.end(),
                [b_new](shared_ptr<browser> el) { return el->open_cmd == b_new->open_cmd; });

            if (b_old_it != old_set.end()) {
                shared_ptr<browser> b_old = *b_old_it;

                // merge user data
                b_new->is_hidden = b_old->is_hidden;
                b_new->sort_order = b_old->sort_order;

                // profiles

                // merge old data into new profiles
                for (shared_ptr<browser_instance> bi_new : b_new->instances) {
                    auto bi_old_it = std::find_if(
                        b_old->instances.begin(), b_old->instances.end(),
                        [bi_new](shared_ptr<browser_instance> el) {return el->id == bi_new->id; });

                    if (bi_old_it == b_old->instances.end()) continue;
                    shared_ptr<browser_instance> bi_old = *bi_old_it;

                    // merge user-defined customisations
                    bi_new->user_arg = bi_old->user_arg;
                    bi_new->user_icon_path = bi_old->user_icon_path;

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
            if(b_custom->is_autodiscovered) continue;

            r.push_back(b_custom);
        }

        browser::sort(r);

        return r;
    }

    size_t browser::index_of(std::vector<std::shared_ptr<bt::browser>>& browsers, std::shared_ptr<bt::browser> b) {
        string id = b->id;
        auto bit = std::find_if(browsers.begin(), browsers.end(),
            [id](const shared_ptr<browser>& i) {return i->id == id; });

        if(bit != browsers.end()) {
            size_t idx = bit - browsers.begin();
            return idx;
        }

        return string::npos;
    }

    void browser::sort(std::vector<std::shared_ptr<browser>>& browsers) {
        std::sort(browsers.begin(), browsers.end(), [](const shared_ptr<browser>& a, const shared_ptr<browser>& b) {
            return a->sort_order < b->sort_order;
        });

        // sort instances by order field
        for(auto& b : browsers) {
            std::sort(b->instances.begin(), b->instances.end(),
                [](const shared_ptr<browser_instance>& a, const shared_ptr<browser_instance>& b) {

                return a->sort_order < b->sort_order;
            });
        }
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
        const std::string& icon_path)
        : b{ b },

        id{ id },
        name{ name },

        launch_arg{ launch_arg },
        icon_path{ icon_path } {
    }

    browser_instance::~browser_instance() {}

    void browser_instance::launch(click_payload up) const {
        string url = up.url;
        string arg = launch_arg;

        if(arg.empty()) {
            arg = url;
        } else {
            size_t pos = arg.find(URL_ARG_NAME);
            if(pos != string::npos) {
                arg.replace(pos, URL_ARG_NAME.size(), url);
            }
        }

        // works in Chrome only
        if(b->get_supports_frameless_windows() && up.app_mode) {
            arg = fmt::format("--app={}", arg);
        }

        // add user-defined attributes
        if(!user_arg.empty()) {
            arg += " ";
            arg += user_arg;
        }

        // if command starts with UWP prefix, launch this as UWP app
        if(b->open_cmd.starts_with(browser::UwpCmdPrefix)) {
            string family_name = b->open_cmd.substr(browser::UwpCmdPrefix.size());
            win32::uwp uwp;
            uwp.launch_uri(family_name, arg);
        } else {
            launch_win32_process_and_foreground(b->open_cmd + " " + arg);
        }
    }

    bool browser_instance::is_match(const click_payload& up, match_rule& mr) const {
        for (const auto& rule : rules) {
            if (rule->is_match(up)) {
                mr = *rule;
                return true;
            }
        }

        return false;
    }

    bool browser_instance::is_match(const click_payload& up, const script_site& ss, match_rule& mr) const {
        for(const auto& rule : rules) {
            if(rule->is_match(up, ss)) {
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

    bool browser_instance::is_singular() const {
        return count_if(b->instances.begin(), b->instances.end(), [](auto i) {return !i->is_incognito; }) == 1;
    }

    std::string browser_instance::get_best_display_name() const {
        if(is_incognito) return fmt::format("Private {}", b->name);

        if(b->is_autodiscovered && is_singular()) return b->name;

        return name;
    }

    std::string browser_instance::get_best_icon_path(bool include_override) const {

        if(include_override && !user_icon_path.empty()) return user_icon_path;

        return icon_path.empty() ? b->open_cmd : icon_path;
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
        for (const string& rule : rules_txt) {
            string clean_rule = rule;
            str::trim(clean_rule);
            if(!clean_rule.empty()) {
                add_rule(rule);
            }
        }
    }

    void browser_instance::launch_win32_process_and_foreground(const std::string& cmdline) const {
        STARTUPINFO si{};
        si.cb = sizeof(si);
        if(launch_hide_ui) {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }
        PROCESS_INFORMATION pi{};
        DWORD pid{0};

        DWORD creation_flags = 0;
        if(launch_hide_ui) {
            creation_flags |= CREATE_NO_WINDOW;
        }

        if(::CreateProcess(nullptr,
            const_cast<wchar_t*>(str::to_wstr(cmdline).c_str()),
            nullptr,
            nullptr,
            false,
            creation_flags,
            nullptr,
            nullptr,
            &si,
            &pi)) {

            // Wait for the process to start before closing the handles,
            // otherwise the process will be terminated (browser will be shown in the background).

            // Wait for 5 seconds maximum
            ::WaitForSingleObject(pi.hProcess, 5000);

            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
        } else {
            string error = win32::kernel::get_last_error_text();
            win32::user::message_box("Browser launch error", fmt::format("Command line: {}.\r\nError: {}", cmdline, error));
        }
    }
}