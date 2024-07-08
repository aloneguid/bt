#include "config.h"
#include "../globals.h"
#include "win32/reg.h"
#include "win32/ole32.h"
#include "win32/shell.h"
#include "str.h"
#include <fmt/core.h>
#include <filesystem>
#include "fss.h"

using namespace std;
namespace fs = std::filesystem;

namespace bt {
    const string FileName = "config.ini";
    const string settings_root = string("SOFTWARE\\") + APP_LONG_NAME;
    const string IIDKeyName = "iid";
    const string BrowserPrefix = "browser";
    const string FirefoxContainerModeKey = "firefox_container_mode";
    const string LogRuleHitsKey = "log_rule_hits";
    const string PersistPopularityKey = "persist_popularity";
    const string ShowHiddenBrowsersKey = "browsers_show_hidden";
    const string UnshortEnabledKey = "unshort_enabled";
    const string PickerSectionName = "picker";
    const string PipelineSectionName = "pipeline";
    const string PipelineSubstKeyName = "subst";

    config::config() : cfg{config::get_data_file_path(FileName)} {
        migrate();
        ensure_instance_id();
        load();
    }

    std::string config::get_iid() {
        return win32::reg::get_value(win32::reg::hive::current_user, settings_root, IIDKeyName);
    }

    std::string config::get_data_file_path(const std::string& name) {
        return fs::exists(fs::path{fss::get_current_dir()} / PortableMarkerName)
            ? (fs::path{fss::get_current_dir()} / name).string()
            : (fs::path{win32::shell::get_local_app_data_path()} / APP_SHORT_NAME / name).string();
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

    void config::load() {
        string v;

        show_hidden_browsers = cfg.get_bool_value(ShowHiddenBrowsersKey, true);
        theme_id = cfg.get_value("theme");
        log_rule_hits = cfg.get_bool_value(LogRuleHitsKey);
        string mode = cfg.get_value(FirefoxContainerModeKey);
        firefox_mode = to_firefox_container_mode(mode);
        default_browser = cfg.get_value("default_browser");

        // picker
        picker_on_key_cs = cfg.get_bool_value("on_key_cs", true, PickerSectionName);
        picker_on_key_ca = cfg.get_bool_value("on_key_ca", false, PickerSectionName);
        picker_on_key_as = cfg.get_bool_value("on_key_as", false, PickerSectionName);
        picker_on_conflict = cfg.get_bool_value("on_conflict", true, PickerSectionName);
        picker_on_no_rule = cfg.get_bool_value("on_no_rule", false, PickerSectionName);
        picker_always = cfg.get_bool_value("always", false, PickerSectionName);

        // pipeline
        pipeline_unwrap_o365 = cfg.get_bool_value("unwrap_o365", true, PipelineSectionName);
        pipeline_unshorten = cfg.get_bool_value("unshorten", true, PipelineSectionName);
        pipeline_substitutions = cfg.get_all_values(PipelineSubstKeyName, PipelineSectionName);

        browsers = load_browsers();
    }

    void config::commit() {
        cfg.set_bool_value(ShowHiddenBrowsersKey, show_hidden_browsers);
        cfg.set_value("theme", theme_id == "follow_os" ? "" : theme_id);
        cfg.set_bool_value(LogRuleHitsKey, log_rule_hits);
        cfg.set_value(FirefoxContainerModeKey, firefox_container_mode_to_string(firefox_mode));
        cfg.set_value("default_browser", default_browser);

        // picker
        cfg.set_bool_value("on_key_cs", picker_on_key_cs, PickerSectionName);
        cfg.set_bool_value("on_key_ca", picker_on_key_ca, PickerSectionName);
        cfg.set_bool_value("on_key_as", picker_on_key_as, PickerSectionName);
        cfg.set_bool_value("on_conflict", picker_on_conflict, PickerSectionName);
        cfg.set_bool_value("on_no_rule", picker_on_no_rule, PickerSectionName);
        cfg.set_bool_value("always", picker_always, PickerSectionName);

        // pipeline
        cfg.set_bool_value("unwrap_o365", pipeline_unwrap_o365, PipelineSectionName);
        cfg.set_bool_value("unshorten", pipeline_unshorten, PipelineSectionName);
        cfg.set_value(PipelineSubstKeyName, pipeline_substitutions, PipelineSectionName);

        save_browsers(browsers);

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
            cfg.set_bool_value("hidden", b->is_hidden, section);

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
                cfg.set_value("user_icon", instance->user_icon_path, section);
            } else {
                // instances
                for(auto& bi : b->instances) {
                    string section = fmt::format("{}:{}:{}", BrowserPrefix, b->id, bi->id);
                    cfg.set_value("name", bi->name, section);
                    cfg.set_value("arg", bi->launch_arg, section);
                    cfg.set_value("user_arg", bi->user_arg, section);
                    cfg.set_value("icon", bi->icon_path, section);
                    cfg.set_value("user_icon", bi->user_icon_path, section);
                    cfg.set_value("subtype", bi->is_incognito ? "incognito" : "", section);
                    cfg.set_value("rule", bi->get_rules_as_text_clean(), section);
                    if(bi->order != 0) cfg.set_value("order", to_string(bi->order), section);
                }
            }
        }
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
            b->is_hidden = cfg.get_bool_value("hidden", false, bsn);

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
                    bi->user_icon_path = cfg.get_value("user_icon", ssn);
                    bi->user_arg = cfg.get_value("user_arg", ssn);
                    bi->is_incognito = p_subtype == "incognito";
                    string s_order = cfg.get_value("order", ssn);
                    if(!s_order.empty()) bi->order = str::to_int(s_order);

                    // rules, if any
                    bi->set_rules_from_text(cfg.get_all_values("rule", ssn));

                    b->instances.push_back(bi);
                }
            } else {
                auto uprof = make_shared<browser_instance>(b, "default", b->name, cfg.get_value("arg", bsn), "");
                uprof->user_icon_path = cfg.get_value("user_icon", bsn);
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