#include "config_custom.h"
#include <memory>
#include "str.h"

using namespace std;
using namespace grey::common;

#define BrowserPrefix "browser"
#define BrowserEngine "engine"
#define IsHidden "hidden"
#define Icon "icon"
#define ItemSortOrder "sort_order"
#define DataPath "data_path"
#define IsAutodiscovered "auto"
#define IsIncognito "incognito"

namespace bt {

    std::string browser_engine_to_string(browser_engine engine) {
        switch(engine) {
            case browser_engine::chromium:    return "chromium";
            case browser_engine::gecko:       return "gecko";
            default:                          return "";
        }
    }

    browser_engine to_browser_engine(const std::string& name) {
        if(name == "chromium")                          return browser_engine::chromium;
        if(name == "gecko")                             return browser_engine::gecko;
        return browser_engine::unknown;
    }

    void save_custom(grey::common::config_handler &cfg, std::vector<std::shared_ptr<bt::browser>> &browsers) {
         // delete all browser sections
        auto section_names = cfg.list_sections();
        for(auto& osn : section_names) {
            if(osn.starts_with(BrowserPrefix))
                cfg.delete_section(osn);
        }

        int order = 0;
        for(auto& b : browsers) {
            b->sort_order = order++;
            string section = format("{}:{}", BrowserPrefix, b->id);
            cfg.save_string_value("name", b->name, section);
            cfg.save_string_value("cmd", b->open_cmd, section);
            cfg.save_bool_value(IsHidden, b->is_hidden, section);
            cfg.save_string_value(Icon, b->icon_path, section);
            cfg.save_bool_value(ItemSortOrder, b->sort_order, section);
            cfg.save_string_value(DataPath, b->data_path, section);
            cfg.save_string_value(BrowserEngine, browser_engine_to_string(b->engine), section);
            cfg.save_bool_value(IsAutodiscovered, b->is_autodiscovered, section);

            // singular user instance
            if(!b->is_autodiscovered && b->instances.size() == 1) {
                auto instance = b->instances[0];
                cfg.save_string_value("arg", instance->launch_arg, section);
                cfg.save_strings_value("rule", instance->get_rules_as_text_clean(), section);
                cfg.save_string_value("user_icon", instance->user_icon_path, section);
                cfg.save_bool_value("hide_ui", instance->launch_hide_ui, section);
            } else {
                // instances
                int sort_order = 0;
                for(auto& bi : b->instances) {
                    bi->sort_order = sort_order++;
                    string section = format("{}:{}:{}", BrowserPrefix, b->id, bi->id);
                    cfg.save_string_value("name", bi->name, section);
                    cfg.save_string_value("arg", bi->launch_arg, section);
                    cfg.save_string_value("user_arg", bi->user_arg, section);
                    cfg.save_string_value("icon", bi->icon_path, section);
                    cfg.save_string_value("user_icon", bi->user_icon_path, section);
                    cfg.save_bool_value(IsIncognito, bi->is_incognito, section);
                    cfg.save_bool_value(IsHidden, bi->is_hidden, section);
                    cfg.save_strings_value("rule", bi->get_rules_as_text_clean(), section);
                    cfg.save_int_value(ItemSortOrder, bi->sort_order, section);
                }
            }
        }
    }

    void load_custom(grey::common::config_handler &cfg, std::vector<std::shared_ptr<bt::browser>> &browsers) {
        browsers.clear();

        auto section_names = cfg.list_sections();
        for(auto& bsn : section_names) {
            vector<string> parts = str::split(bsn, ":");
            if(parts[0] != BrowserPrefix || parts.size() != 2) continue;

            string b_id = parts[1];
            auto b = make_shared<browser>(
                b_id,
                cfg.load_string_value("name", "", bsn),
                cfg.load_string_value("cmd", "", bsn)
            );

            b->engine = to_browser_engine(cfg.load_string_value(BrowserEngine, "", bsn));
            b->is_hidden = cfg.load_bool_value(IsHidden, false, bsn);
            b->icon_path = cfg.load_string_value(Icon, "", bsn);
            b->sort_order = cfg.load_int_value(ItemSortOrder, 0, bsn);
            b->data_path = cfg.load_string_value(DataPath, "", bsn);
            b->is_autodiscovered = cfg.load_bool_value(IsAutodiscovered, false, bsn);

            if(b->is_autodiscovered) {

                // profiles, if any
                string profile_prefix = format("{}:{}", BrowserPrefix, b_id);
                for(auto& ssn : section_names) {
                    vector<string> parts = str::split(ssn, ":");
                    if(parts[0] != BrowserPrefix || parts.size() != 3 || parts[1] != b_id) continue;

                    string p_sys_name = parts[2];

                    auto bi = make_shared<browser_instance>(
                        b,
                        p_sys_name,
                        cfg.load_string_value("name", "", ssn),
                        cfg.load_string_value("arg", "", ssn),
                        cfg.load_string_value("icon", "", ssn));

                    bi->user_icon_path = cfg.load_string_value("user_icon", "", ssn);
                    bi->user_arg = cfg.load_string_value("user_arg", "", ssn);
                    bi->is_incognito = cfg.load_bool_value(IsIncognito, false, ssn);
                    bi->is_hidden = cfg.load_bool_value(IsHidden, false, ssn);
                    bi->sort_order = cfg.load_int_value(ItemSortOrder, 0, ssn);

                    // rules, if any
                    bi->set_rules_from_text(cfg.load_strings_value("rule", {}, ssn));

                    b->instances.push_back(bi);
                }
            } else {
                auto uprof = make_shared<browser_instance>(b, "default", b->name, cfg.load_string_value("arg", bsn), "");
                uprof->user_icon_path = cfg.load_string_value("user_icon", "", bsn);
                uprof->set_rules_from_text(cfg.load_strings_value("rule", {}, bsn));
                uprof->launch_hide_ui = cfg.load_bool_value("hide_ui", false, bsn);
                b->instances.push_back(uprof);
            }

            // skip browsers with no instances (could be a bug or user's dirty hands)
            if(!b->instances.empty()) {
                browsers.push_back(b);
            }
        }

        browser::sort(browsers);
    }
}
