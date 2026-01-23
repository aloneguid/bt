#include "config.h"
#include "config.h"
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
    #define ConfigFileName "config.ini"
    #define settings_root "SOFTWARE\\" APP_LONG_NAME
    #define IIDKeyName "iid"
    #define BrowserPrefix "browser"
    #define IsHidden "hidden"
    #define Icon "icon"
    #define ItemSortOrder "sort_order"
    #define DataPath "data_path"
    #define FirefoxContainerModeKey "firefox_container_mode"
    #define DefaultProfileKey "default_profile"
    #define ToastOnOpenKey "toast_on_open"
    #define ToastVisibleSecsKey "toast_visible_secs"
    #define ToastBorderWidthKey "toast_border_width"
    #define IconOverlayKey "icon_overlay"
    #define LogRuleHitsKey "log_rule_hits"
    #define LogAppKey "log_app"
    #define PersistPopularityKey "persist_popularity"
    #define ShowHiddenBrowsersKey "browsers_show_hidden"
    #define DiscoverFirefoxContainersKey "firefox_containers"
    #define UnshortEnabledKey "unshort_enabled"
    #define PickerSectionName "picker"
    #define PickerOnKeyCS "on_key_cs"
    #define PickerOnKeyCA "on_key_ca"
    #define PickerOnKeyAS "on_key_as"
    #define PickerOnKeyCL "on_key_cl"
    #define PickerOnConflict "on_conflict"
    #define PickerCloseOnFocusLoss "close_on_focus_loss"
    #define PickerAlwaysOnTop "always_on_top"
    #define PickerIconSize "icon_size"
    #define PickerItemPadding "item_padding"
    #define PickerInactiveItemAlpha "inactive_item_alpha"
    #define PickerShowKeyHints "show_key_hints"
    #define PickerBorderWidth "border_width"
    #define PickerShowNativeChrome "show_native_chrome"
    #define PipelineSectionName "pipeline"
    #define PipelineSubstKeyName "subst"
    #define PipelineUnwrapO365Key "unwrap_o365"
    #define PipelineUnshortenKey "unshorten"
    #define PipelineSubstituteKey "substitute"
    #define PipelineScriptKey "script"
    #define PipeVisualiserSectionName "pipevis"

    config::config() : cfg{config::get_data_file_path(ConfigFileName)} {
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
        discover_firefox_containers = cfg.get_bool_value(DiscoverFirefoxContainersKey, false);

        theme_id = cfg.get_value("theme");
        log_rule_hits = cfg.get_bool_value(LogRuleHitsKey);
        log_app = cfg.get_bool_value(LogAppKey);
        string mode = cfg.get_value(FirefoxContainerModeKey);
        default_profile_long_id = cfg.get_value(DefaultProfileKey);
        toast_on_open = cfg.get_bool_value(ToastOnOpenKey, true);
        toast_visible_secs = cfg.get_int_value(ToastVisibleSecsKey, 3);
        toast_border_width = cfg.get_int_value(ToastBorderWidthKey, 1);
        icon_overlay = to_icon_overlay_mode(cfg.get_value(IconOverlayKey));

        // picker
        picker_on_key_cs = cfg.get_bool_value(PickerOnKeyCS, true, PickerSectionName);
        picker_on_key_ca = cfg.get_bool_value(PickerOnKeyCA, false, PickerSectionName);
        picker_on_key_as = cfg.get_bool_value(PickerOnKeyAS, false, PickerSectionName);
        picker_on_key_cl = cfg.get_bool_value(PickerOnKeyCL, false, PickerSectionName);
        picker_on_conflict = cfg.get_bool_value(PickerOnConflict, true, PickerSectionName);
        picker_on_no_rule = cfg.get_bool_value("on_no_rule", false, PickerSectionName);
        picker_always = cfg.get_bool_value("always", false, PickerSectionName);
        picker_close_on_focus_loss = cfg.get_bool_value(PickerCloseOnFocusLoss, false, PickerSectionName);
        picker_always_on_top = cfg.get_bool_value(PickerAlwaysOnTop, false, PickerSectionName);
        picker_icon_size = cfg.get_float_value(PickerIconSize, 32.0f, PickerSectionName);
        picker_item_padding = cfg.get_float_value(PickerItemPadding, 10.0f, PickerSectionName);
        picker_inactive_item_alpha = cfg.get_float_value(PickerInactiveItemAlpha, 0.4f, PickerSectionName);
        picker_show_key_hints = cfg.get_bool_value(PickerShowKeyHints, true, PickerSectionName);
        picker_border_width = cfg.get_int_value(PickerBorderWidth, 1, PickerSectionName);
        picker_show_native_chrome = cfg.get_bool_value(PickerShowNativeChrome, false, PickerSectionName);

        // pipeline
        pipeline_unwrap_o365 = cfg.get_bool_value(PipelineUnwrapO365Key, true, PipelineSectionName);
        pipeline_unshorten = cfg.get_bool_value(PipelineUnshortenKey, true, PipelineSectionName);
        pipeline_substitute = cfg.get_bool_value(PipelineSubstituteKey, true, PipelineSectionName);
        pipeline_substitutions = cfg.get_all_values(PipelineSubstKeyName, PipelineSectionName);
        pipeline_script = cfg.get_bool_value(PipelineScriptKey, true, PipelineSectionName);

        // pipe visualiser
        pv_last_url = cfg.get_value("last_url", PipeVisualiserSectionName);
        pv_last_wt = cfg.get_value("last_wt", PipeVisualiserSectionName);
        pv_last_pn = cfg.get_value("last_pn", PipeVisualiserSectionName);

        browsers = load_browsers();
    }

    void config::commit() {
        cfg.set_value(ShowHiddenBrowsersKey, show_hidden_browsers);
        cfg.set_value(DiscoverFirefoxContainersKey, discover_firefox_containers);
        cfg.set_value("theme", theme_id == "follow_os" ? "" : theme_id);
        cfg.set_value(LogRuleHitsKey, log_rule_hits);
        cfg.set_value(LogAppKey, log_app);
        cfg.set_value(DefaultProfileKey, default_profile_long_id);
        cfg.set_value(ToastOnOpenKey, toast_on_open);
        cfg.set_value(ToastVisibleSecsKey, toast_visible_secs);
        cfg.set_value(ToastBorderWidthKey, toast_border_width);
        cfg.set_value(IconOverlayKey, icon_overlay_mode_to_string(icon_overlay));

        // picker
        cfg.set_value(PickerOnKeyCS, picker_on_key_cs, PickerSectionName);
        cfg.set_value(PickerOnKeyCA, picker_on_key_ca, PickerSectionName);
        cfg.set_value(PickerOnKeyAS, picker_on_key_as, PickerSectionName);
        cfg.set_value(PickerOnKeyCL, picker_on_key_cl, PickerSectionName);
        cfg.set_value(PickerOnConflict, picker_on_conflict, PickerSectionName);
        cfg.set_value("on_no_rule", picker_on_no_rule, PickerSectionName);
        cfg.set_value("always", picker_always, PickerSectionName);
        cfg.set_value(PickerCloseOnFocusLoss, picker_close_on_focus_loss, PickerSectionName);
        cfg.set_value(PickerAlwaysOnTop, picker_always_on_top, PickerSectionName);
        cfg.set_value(PickerIconSize, picker_icon_size, PickerSectionName);
        cfg.set_value(PickerItemPadding, picker_item_padding, PickerSectionName);
        cfg.set_value(PickerInactiveItemAlpha, picker_inactive_item_alpha, PickerSectionName);
        cfg.set_value(PickerShowKeyHints, picker_show_key_hints, PickerSectionName);
        cfg.set_value(PickerBorderWidth, picker_border_width, PickerSectionName);
        cfg.set_value(PickerShowNativeChrome, picker_show_native_chrome, PickerSectionName);

        // pipeline
        cfg.set_value(PipelineUnwrapO365Key, pipeline_unwrap_o365, PipelineSectionName);
        cfg.set_value(PipelineUnshortenKey, pipeline_unshorten, PipelineSectionName);
        cfg.set_value(PipelineSubstituteKey, pipeline_substitute, PipelineSectionName);
        cfg.set_value(PipelineSubstKeyName, pipeline_substitutions, PipelineSectionName);
        cfg.set_value(PipelineScriptKey, pipeline_script, PipelineSectionName);

        // pipe visualiser
        cfg.set_value("last_url", pv_last_url, PipeVisualiserSectionName);
        cfg.set_value("last_wt", pv_last_wt, PipeVisualiserSectionName);
        cfg.set_value("last_pn", pv_last_pn, PipeVisualiserSectionName);

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

        int order = 0;
        for(auto& b : browsers) {
            b->sort_order = order++;
            string section = fmt::format("{}:{}", BrowserPrefix, b->id);
            cfg.set_value("name", b->name, section);
            cfg.set_value("cmd", b->open_cmd, section);
            cfg.set_value(IsHidden, b->is_hidden, section);
            cfg.set_value(Icon, b->icon_path, section);
            cfg.set_value(ItemSortOrder, b->sort_order, section);
            cfg.set_value(DataPath, b->data_path, section);

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
                cfg.set_value("hide_ui", instance->launch_hide_ui, section);
            } else {
                // instances
                int sort_order = 0;
                for(auto& bi : b->instances) {
                    bi->sort_order = sort_order++;
                    string section = fmt::format("{}:{}:{}", BrowserPrefix, b->id, bi->id);
                    cfg.set_value("name", bi->name, section);
                    cfg.set_value("arg", bi->launch_arg, section);
                    cfg.set_value("user_arg", bi->user_arg, section);
                    cfg.set_value("icon", bi->icon_path, section);
                    cfg.set_value("user_icon", bi->user_icon_path, section);
                    cfg.set_value("subtype", bi->is_incognito ? "incognito" : "", section);
                    cfg.set_value(IsHidden, bi->is_hidden, section);
                    cfg.set_value("rule", bi->get_rules_as_text_clean(), section);
                    cfg.set_value(ItemSortOrder, bi->sort_order, section);
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
            b->is_hidden = cfg.get_bool_value(IsHidden, false, bsn);
            b->icon_path = cfg.get_value(Icon, bsn);
            b->sort_order = cfg.get_int_value(ItemSortOrder, 0, bsn);
            b->data_path = cfg.get_value(DataPath, bsn);

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
                    bi->is_hidden = cfg.get_bool_value(IsHidden, false, ssn);
                    bi->sort_order = cfg.get_int_value(ItemSortOrder, 0, ssn);

                    // rules, if any
                    bi->set_rules_from_text(cfg.get_all_values("rule", ssn));

                    b->instances.push_back(bi);
                }
            } else {
                auto uprof = make_shared<browser_instance>(b, "default", b->name, cfg.get_value("arg", bsn), "");
                uprof->user_icon_path = cfg.get_value("user_icon", bsn);
                uprof->set_rules_from_text(cfg.get_all_values("rule", bsn));
                uprof->launch_hide_ui = cfg.get_bool_value("hide_ui", false, bsn);
                b->instances.push_back(uprof);
            }

            // skip browsers with no instances (could be a bug or user's dirty hands)
            if(!b->instances.empty()) {
                r.push_back(b);
            }
        }

        browser::sort(r);

        return r;
    }

    string config::get_absolute_path() {
        return cfg.get_absolute_path();
    }

    string config::get_flag(const std::string& name) {
        return cfg.get_value(fmt::format("flag_{}", name));
    }

    std::string bt::config::icon_overlay_mode_to_string(icon_overlay_mode mode) {
        switch(mode) {
            case icon_overlay_mode::profile_on_browser:     return "profile_on_browser";
            case icon_overlay_mode::browser_on_profile:     return "browser_on_profile";
            case icon_overlay_mode::browser_only:           return "browser_only";
            case icon_overlay_mode::profile_only:           return "profile_only";
            default:                                        return "profile_on_browser";
        }
    }

    icon_overlay_mode config::to_icon_overlay_mode(const std::string& name) {
        if(name == "profile_on_browser")   return icon_overlay_mode::profile_on_browser;
        if(name == "browser_on_profile")   return icon_overlay_mode::browser_on_profile;
        if(name == "browser_only")         return icon_overlay_mode::browser_only;
        if(name == "profile_only")         return icon_overlay_mode::profile_only;
        return icon_overlay_mode::profile_on_browser;
    }

}