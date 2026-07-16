#include "browser.h"
#include "match_rule.h"
#include <filesystem>
#include <algorithm>
#include "str.h"
#include <format>
#include "process.h"
#include "common/stl.hpp"
#include <ranges>

#if PLATFORM_WINDOWS
#include "win32/shell.h"
#include "win32/os.h"
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
        const std::string &name,
        const std::string &open_cmd)
        : name{name}, open_cmd{open_cmd} {
        str::trim(this->name);
    }

    void browser::launch(click_payload up, const browser_profile &profile) const {
        string url = up.url;
        string arg = profile.launch_arg;

        if(arg.empty()) {
            arg = url;
        } else {
            size_t pos = arg.find(URL_ARG_NAME);
            if(pos != string::npos) {
                arg.replace(pos, URL_ARG_NAME.size(), url);
            }
        }

        // works in Chrome only
        if(get_supports_frameless_windows() && up.app_mode) {
            arg = format("--app={}", arg);
        }

        // add user-defined attributes
        if(!profile.user_arg.empty()) {
            arg += " ";
            arg += profile.user_arg;
        }

        launch_process(open_cmd + " " + arg, profile);
    }

    void browser::launch_process(const std::string &cmdline, const browser_profile &profile) const {
#if PLATFORM_WINDOWS
        STARTUPINFO si{};
        si.cb = sizeof(si);
        if(profile.launch_hide_ui) {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }
        PROCESS_INFORMATION pi{};
        DWORD pid{0};

        DWORD creation_flags = 0;
        if(profile.launch_hide_ui) {
            creation_flags |= CREATE_NO_WINDOW;
        }

        if(::CreateProcess(nullptr,
                           const_cast<wchar_t *>(str::to_wstr(cmdline).c_str()),
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


    size_t browser::get_total_rule_count() const {
        size_t r{0};
        for(auto &i: profiles) {
            r += i.rules.size();
        }
        return r;
    }

    std::string browser::get_best_icon_path() const {
        if(!icon_path.empty()) return icon_path;

        if(engine == browser_engine::generic) {
            if(!profiles.empty()) {
                return get_best_icon_path(profiles[0]);
            }
        }

        return open_cmd;
    }

    bool browser::is_default() const {
        for(const auto &i: profiles) {
            if(i.is_default) return true;
        }
        return false;
    }

    std::vector<browser_match_result> browser::match(
        const std::vector<browser> &browsers,
        const click_payload &up,
        const script_site &script) {
        vector<browser_match_result> r;

        // which browser should we use?
        // i'm not sure this is correct
        for(const browser& browser: browsers) {
            for(auto [i, profile] : browser.profiles | std::views::enumerate) {
                match_rule mr;
                if(profile.is_match(up, script, mr)) {
                    r.emplace_back(profile_selection(browser, i), mr);
                }
            }
        }

        // if nothing matched, used default profile
        if(r.empty()) {
            auto default_profile = get_default(browsers);
            if(default_profile) {
                match_rule fbr{"default"};
                fbr.is_fallback = true;
                r.emplace_back(*default_profile, fbr);
            }
        }

        return r;
    }

    optional<profile_selection> browser::get_default(const std::vector<browser> &browsers) {
        optional<profile_selection> fallback_candidate;

        for(auto &b: browsers) {
            for(auto [i, profile] : b.profiles | std::views::enumerate) {
                if(!fallback_candidate) fallback_candidate = profile_selection(b, i);
                if(profile.is_default) return profile_selection(b, i);
            }
        }

        return fallback_candidate;
    }

    void browser::set_default(std::vector<browser> &browsers,
                              const browser_profile &profile) {
        for(auto &b: browsers) {
            for(browser_profile &p: b.profiles) {
                p.is_default = p == profile;
            }
        }
    }

    std::vector<browser> browser::merge(
        std::vector<browser> &new_set, std::vector<browser> &old_set) {
        vector<browser> r;

        for(browser &b_new: new_set) {
            // find corresponding browser by open_cmd
            auto b_old_it = std::find_if(
                old_set.begin(), old_set.end(),
                [b_new](browser &el) { return el.open_cmd == b_new.open_cmd; });

            if(b_old_it != old_set.end()) {
                browser b_old = *b_old_it;

                // merge user data
                b_new.is_hidden = b_old.is_hidden;

                // profiles

                // merge old data into new profiles
                for(browser_profile &bi_new: b_new.profiles) {
                    auto bi_old_it = std::find_if(
                        b_old.profiles.begin(), b_old.profiles.end(),
                        [bi_new](const browser_profile &el) { return el.name == bi_new.name; });

                    if(bi_old_it == b_old.profiles.end()) continue;
                    const browser_profile &bi_old = *bi_old_it;

                    // merge user-defined customisations
                    bi_new.user_arg = bi_old.user_arg;
                    bi_new.user_icon_path = bi_old.user_icon_path;

                    // merge rules
                    for(auto &rule: bi_old.rules) {
                        bi_new.rules.push_back(rule);
                    }
                }
            }

            r.push_back(b_new);
        }

        // add missing browsers
        for(browser &b_custom: old_set) {
            if(b_custom.management == management_extent::full) continue;

            r.push_back(b_custom);
        }

        return r;
    }

    size_t browser::index_of(std::vector<browser> &browsers, browser &b) {
        for(size_t i = 0; i < browsers.size(); ++i) {
            if(browsers[i] == b) return i;
        }

        return string::npos;
    }

    std::string browser::get_best_display_name(const browser_profile &profile) const {
        if(profile.is_incognito) return "Incognito";
        return profile.name;
    }

    std::string browser::get_best_icon_path(const browser_profile &profile, bool include_override) const {
        if(include_override && !profile.user_icon_path.empty()) return profile.user_icon_path;
        if(!profile.icon_path.empty()) return profile.icon_path;
        if(!icon_path.empty()) return icon_path;
        return open_cmd;
    }

    std::string browser::get_image_name(const std::string &open_cmd) {
        if(open_cmd.empty()) return open_cmd;
        return fs::path{open_cmd}.filename().replace_extension().string();
    }

    browser_profile::browser_profile(
        const std::string &name,
        const std::string &launch_arg,
        const std::string &icon_path)
        : name{name},

          launch_arg{launch_arg},
          icon_path{icon_path} {
    }

    bool browser_profile::operator==(const browser_profile &other) const {
        return name == other.name &&
               launch_arg == other.launch_arg &&
               icon_path == other.icon_path &&
               user_arg == other.user_arg &&
               is_hidden == other.is_hidden &&
               icon_path == other.icon_path &&
               user_icon_path == other.user_icon_path &&
               launch_hide_ui == other.launch_hide_ui &&
               is_default == other.is_default &&
               has_firefox_ouic_addon == other.has_firefox_ouic_addon &&
               rules == other.rules;
    }

    browser_profile::~browser_profile() {
    }

    bool browser_profile::is_match(const click_payload &up, match_rule &mr) const {
        for(const auto &rule: rules) {
            if(rule.is_match(up)) {
                mr = rule;
                return true;
            }
        }

        return false;
    }

    bool browser_profile::is_match(const click_payload &up, const script_site &ss, match_rule &mr) const {
        for(const auto &rule: rules) {
            if(rule.is_match(up, ss)) {
                mr = rule;
                return true;
            }
        }

        return false;
    }

    bool browser_profile::add_rule(const std::string &rule_text) {
        match_rule new_rule{rule_text};

        for(const auto &rule: rules) {
            if(rule == new_rule)
                return false;
        }

        rules.push_back(new_rule);

        return true;
    }

    void browser_profile::delete_rule(const std::string &rule_text) {
        std::erase_if(rules, [rule_text](auto r) { return r.value == rule_text; });
    }
}
