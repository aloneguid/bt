#include "config.h"
#include "../globals.h"
#include "win32/reg.h"
#include "win32/ole32.h"
#include "win32/shell.h"
#include "str.h"
#include "config/config.h"
#include <fmt/core.h>
#include <filesystem>
#include "fss.h"

using namespace std;
namespace fs = std::filesystem;

namespace bt {
    const string settings_root = string("SOFTWARE\\") + APP_LONG_NAME;
    const string IIDKeyName = "iid";
    const string BrowserPrefix = "browser";
    const string FirefoxContainerModeKey = "firefox_container_mode";

    common::config cfg{ 
        fs::exists(fs::path{fss::get_current_dir()} / ".portable")
            ? (fs::path{fss::get_current_dir()} / "config.ini").string()
            : (fs::path{win32::shell::get_local_app_data_path()} / APP_SHORT_NAME / "config.ini").string()
    };

    config config::i;

    config::config() {
        migrate();
        ensure_instance_id();
    }

    std::string config::get_iid() {
        return win32::reg::get_value(win32::reg::hive::current_user, settings_root, IIDKeyName);
    }

    void config::set_theme(const std::string& id) {
        cfg.set_value("theme", id == "follow_os" ? "" : id);
        cfg.commit();
    }

    std::string config::get_theme() {
        return cfg.get_value("theme");
    }

    void config::ensure_instance_id() {
        // ensure instance ID is remembered
        string iid = cfg.get_value(IIDKeyName);

        // if found in .ini, remove it
        if(!iid.empty()) {
            cfg.delete_key(IIDKeyName);
            win32::reg::set_value(win32::reg::hive::current_user, settings_root, iid, IIDKeyName);
        } else {
            iid = win32::reg::get_value(win32::reg::hive::current_user, settings_root, IIDKeyName);
            if(iid.empty()) {
                iid = win32::ole32::create_guid();
                win32::reg::set_value(win32::reg::hive::current_user, settings_root, iid, IIDKeyName);
            }
        }
    }

    void config::migrate() {
        bool is_dirty{false};

        // delete section [browser:user]
        auto section_names = cfg.list_sections();
        const string LegacyBUSection = "browser:user";
        if(std::find(section_names.begin(), section_names.end(), LegacyBUSection) != section_names.end()) {
            is_dirty = true;
            cfg.delete_section(LegacyBUSection);
        }

        // migrate sections that look like [browser:user:6e182f86-d748-451e-b966-ffd60de25957] to new format
        section_names = cfg.list_sections();
        for(auto ssn : section_names) {
            if(!ssn.starts_with("browser:user:")) continue;

            vector<string> parts = str::split(ssn, ":");
            if(parts.size() != 3) continue;

            is_dirty = true;

            // read old
            string id = parts[2];
            string name = cfg.get_value("name", ssn);
            string arg = cfg.get_value("arg", ssn);
            string cmd = cfg.get_value("cmd", ssn);
            auto rules = cfg.get_all_values("rule", ssn);
            cfg.delete_section(ssn);

            // write new
            string section = fmt::format("{}:{}", BrowserPrefix, id);
            cfg.set_value("name", name, section);
            cfg.set_value("arg", arg, section);
            cfg.set_value("cmd", cmd, section);
            cfg.set_value("subtype", "user", section);
            cfg.set_value("rule", rules, section);
        }

        if(is_dirty) {
            cfg.commit();
        }
    }

    void config::set_picker_enabled(bool enabled) {
        cfg.set_value("use_picker", enabled ? "y" : "n");
        cfg.commit();
    }

    bool config::get_picker_enabled() {
        return cfg.get_value("use_picker") != "n";
    }

    void config::set_picker_hotkey(const std::string& hotkey) {
        cfg.set_value("picker_hotkey", hotkey);
        cfg.commit();
    }

    std::string config::get_picker_hotkey() {
        string v = cfg.get_value("picker_hotkey");
        if(v.empty()) v = "cs";
        return v;
    }

    void config::set_open_method(const std::string& method_name) {
        if(method_name == "decide") {
            cfg.delete_key("open_method");
        } else {
            cfg.set_value("open_method", method_name);
        }
        cfg.commit();
    }

    std::string config::get_open_method() {
        auto m = cfg.get_value("open_method");
        return m.empty() ? "decide" : m;
    }

    void config::set_fallback(const string& long_sys_name) {
        cfg.set_value("fallback", long_sys_name);
        cfg.commit();
    }

    string config::get_fallback_long_sys_name() {
        return cfg.get_value("fallback");
    }

    int config::get_popularity(const std::string& long_sys_name) {
        string v = cfg.get_value(long_sys_name, "popularity");
        return str::to_int(v);
    }

    void config::set_popularity(const std::string& long_sys_name, int value) {
        cfg.set_value(long_sys_name, std::to_string(value), "popularity");
        cfg.commit();
    }

    bool config::get_notify_on_rule_hit() {
        return cfg.get_value("notify_on_rule_hit") == "y";
    }

    void config::set_notify_on_rule_hit(bool notify) {
        cfg.set_value("notify_on_rule_hit", notify ? "y" : "");
        cfg.commit();
    }

    firefox_container_mode config::get_firefox_container_mode() {
        string mode = cfg.get_value(FirefoxContainerModeKey);
        return to_firefox_container_mode(mode);
    }

    void config::set_firefox_container_mode(firefox_container_mode mode) {
        cfg.set_value(FirefoxContainerModeKey, firefox_container_mode_to_string(mode));
        cfg.commit();
    }

    void config::save_browsers(std::vector<std::shared_ptr<browser>> browsers) {

        // delete all browser sections
        auto section_names = cfg.list_sections();
        for(auto& osn : section_names) {
            if(osn.starts_with(BrowserPrefix))
                cfg.delete_section(osn);
        }

        for(auto& b : browsers) {
            string section = fmt::format("{}:{}", BrowserPrefix, b->id);
            cfg.set_value("name", b->name, section);
            cfg.set_value("cmd", b->open_cmd, section);

            string subtype;
            if(b->is_system) {
                if(b->is_firefox) subtype = "firefox";
                else if(b->is_chromium) subtype = "chromium";
            } else {
                subtype = "user";
            }
            cfg.set_value("subtype", subtype, section);

            // singular user instance
            if(!b->is_system && b->instances.size() == 1) {
                auto instance = b->instances[0];
                cfg.set_value("arg", instance->launch_arg, section);
                cfg.set_value("rule", instance->get_rules_as_text_clean(), section);
            } else {
                // instances
                for(auto& bi : b->instances) {
                    string section = fmt::format("{}:{}:{}", BrowserPrefix, b->id, bi->id);
                    cfg.set_value("name", bi->name, section);
                    cfg.set_value("arg", bi->launch_arg, section);
                    cfg.set_value("icon", bi->icon_path, section);
                    cfg.set_value("subtype", bi->is_incognito ? "incognito" : "", section);
                    cfg.set_value("rule", bi->get_rules_as_text_clean(), section);
                    if(bi->order != 0) cfg.set_value("order", to_string(bi->order), section);
                }
            }
        }
        cfg.commit();
    }

    std::vector<std::shared_ptr<browser>> config::load_browsers() {

        vector<shared_ptr<browser>> r;

        auto section_names = cfg.list_sections();
        for(auto& bsn : section_names) {
            vector<string> parts = str::split(bsn, ":");
            if(parts[0] != BrowserPrefix || parts.size() != 2) continue;

            string subtype = cfg.get_value("subtype", bsn);
            string b_id = parts[1];

            auto b = make_shared<browser>(
                b_id,
                cfg.get_value("name", bsn),
                cfg.get_value("cmd", bsn),
                subtype != "user"
            );

            b->is_firefox = subtype == "firefox";
            b->is_chromium = subtype == "chromium";


            if(b->is_system) {

                // profiles, if any
                string profile_prefix = fmt::format("{}:{}", BrowserPrefix, b_id);
                for(auto& ssn : section_names) {
                    vector<string> parts = str::split(ssn, ":");
                    if(parts[0] != BrowserPrefix || parts.size() != 3 || parts[1] != b_id) continue;

                    string p_sys_name = parts[2];
                    string p_subtype = cfg.get_value("subtype", ssn);

                    auto bi = make_shared<browser_instance>(
                        b,
                        p_sys_name,
                        cfg.get_value("name", ssn),
                        cfg.get_value("arg", ssn),
                        cfg.get_value("icon", ssn));
                    bi->is_incognito = p_subtype == "incognito";
                    string s_order = cfg.get_value("order", ssn);
                    if(!s_order.empty()) bi->order = str::to_int(s_order);

                    // rules, if any
                    bi->set_rules_from_text(cfg.get_all_values("rule", ssn));

                    b->instances.push_back(bi);
                }
            } else {
                auto uprof = make_shared<browser_instance>(b, "default", b->name, cfg.get_value("arg", bsn), "");
                uprof->set_rules_from_text(cfg.get_all_values("rule", bsn));
                b->instances.push_back(uprof);
            }

            // skip browsers with no instances (could be a bug or user's dirty hands)
            if(!b->instances.empty()) {
                r.push_back(b);
            }
        }

        // sort by browser name alphabetically
        std::sort(r.begin(), r.end(), [](const shared_ptr<browser>& a, const shared_ptr<browser>& b) {

            string sa = a->name;
            string sb = b->name;
            if(!a->is_system) sa = "z" + sa;
            if(!b->is_system) sb = "z" + sb;
            str::lower(sa);
            str::lower(sb);
            return sa < sb;
        });

        // sort instances by order field
        for(auto& b : r) {
            std::sort(b->instances.begin(), b->instances.end(),
                [](const shared_ptr<browser_instance>& a, const shared_ptr<browser_instance>& b) {

                return a->order < b->order;
            });
        }

        return r;
    }

    std::chrono::system_clock::time_point config::get_last_update_check_time() {
        string s = cfg.get_value("last_update_check");
        auto ll = str::to_long_long(s);
        return std::chrono::system_clock::from_time_t(ll);
    }

    void config::set_last_update_check_time_to_now() {
        auto tp = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(tp);
        string s = std::to_string(tt);
        cfg.set_value("last_update_check", s);
    }

    string config::get_absolute_path() {
        return cfg.get_absolute_path();
    }

    string config::get_flag(const std::string& name) {
        return cfg.get_value(fmt::format("flag_{}", name));
    }

    std::string config::firefox_container_mode_to_string(firefox_container_mode mode) {
        switch(mode) {
            case bt::firefox_container_mode::bt:    return "bt";
            case bt::firefox_container_mode::ouic:  return "ouic";
            default:                                return "";
        }
    }
    firefox_container_mode config::to_firefox_container_mode(const std::string& name) {
        if(name == "bt")    return firefox_container_mode::bt;
        if(name == "ouic")  return firefox_container_mode::ouic;

        return firefox_container_mode::off;
    }
}