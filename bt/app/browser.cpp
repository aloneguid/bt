#include "browser.h"
#include "match_rule.h"
#include <filesystem>
#include <algorithm>
#include "str.h"
#include <format>
#include "process.h"

#if PLATFORM_WINDOWS
#include "win32/shell.h"
#include "win32/os.h"
#include "win32/uwp.h"
#include "win32/user.h"
#endif

namespace fs = std::filesystem;
using namespace std;
using namespace grey::common;

namespace bt {
#if PLATFORM_WINDOWS
    const string lad = win32::shell::get_local_app_data_path();
#endif

    browser::browser(
        const std::string& name,
        const std::string& open_cmd)
        : name{ name }, open_cmd{ open_cmd }
    {
        str::trim(this->name);
    }

    bool browser::operator==(const browser& other) const {
        return open_cmd == other.open_cmd && data_path == other.data_path;
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

    bool browser::is_default() const {
        for(const auto i : instances) {
            if(i->is_default) return true;
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

    std::vector<browser_match_result> browser::match(
        const std::vector<shared_ptr<browser>>& browsers,
        const click_payload& up,
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
            r.emplace_back(get_default(browsers), fbr);
        }

        // sort by priority, descending
        if(r.size() > 1) {
            std::sort(r.begin(), r.end(), [](const browser_match_result& a, const browser_match_result& b) {
                return a.rule.priority > b.rule.priority;
            });
        }

        return r;
    }

    shared_ptr<browser_instance> browser::get_default(const std::vector<shared_ptr<browser>>& browsers) {
        if(browsers.empty()) return nullptr;

        for (auto b : browsers) {
            for (auto& p : b->instances) {
                if(p->is_default) return p;
            }
        }

        auto b = browsers.front();
        if(b->instances.empty()) return nullptr;
        return b->instances.front();
    }

    void browser::set_default(const std::vector<std::shared_ptr<browser>> &browsers,
        const std::shared_ptr<browser_instance> &bi) {
        for (auto b : browsers) {
            for (auto& p : b->instances) {
                p->is_default = *p == *bi;
            }
        }
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

                // profiles

                // merge old data into new profiles
                for (shared_ptr<browser_instance> bi_new : b_new->instances) {
                    auto bi_old_it = std::find_if(
                        b_old->instances.begin(), b_old->instances.end(),
                        [bi_new](shared_ptr<browser_instance> el) {return *el == *bi_new; });

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

        return r;
    }

    size_t browser::index_of(std::vector<std::shared_ptr<bt::browser>>& browsers, std::shared_ptr<bt::browser> b) {
        for(size_t i = 0; i < browsers.size(); ++i) {
            if(*browsers[i] == *b) return i;
        }

        return string::npos;
    }

    std::string browser::get_image_name(const std::string& open_cmd) {
        if(open_cmd.empty()) return open_cmd;
        return fs::path{open_cmd}.filename().replace_extension().string();
    }

    browser_instance::browser_instance(
        shared_ptr<browser> b,
        const std::string& name,
        const std::string& launch_arg,
        const std::string& icon_path)
        : b{ b },

        name{ name },

        launch_arg{ launch_arg },
        icon_path{ icon_path } {
    }

    bool browser_instance::operator==(const browser_instance &other) const {
        return *b == *other.b && name == other.name;
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
            arg = format("--app={}", arg);
        }

        // add user-defined attributes
        if(!user_arg.empty()) {
            arg += " ";
            arg += user_arg;
        }

        launch_process(b->open_cmd + " " + arg);
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

    std::string browser_instance::get_best_display_name() const {
        if(is_incognito) return format("Private {}", b->name);
        return name;
    }

    std::string browser_instance::get_best_icon_path(bool include_override) const {

        if(include_override && !user_icon_path.empty()) return user_icon_path;

        return icon_path.empty() ? b->open_cmd : icon_path;
    }

    void browser_instance::launch_process(const std::string& cmdline) const {
#if PLATFORM_WINDOWS
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
            string error = win32::os::get_last_error_text();
            win32::user::message_box("Browser launch error", format("Command line: {}.\r\nError: {}", cmdline, error));
        }
#else
        process::start(cmdline);
#endif
    }
}