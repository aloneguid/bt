#pragma once
#include <string>
#include <vector>
#include "model.h"
#include "common/state_handler.hpp"
#include <magic_enum/magic_enum.hpp>
#include "app/browser.h"

namespace bt {
    class toast_state {
    public:
        bool enabled;
        int visible_seconds;
        int border_width;

        bool operator==(const toast_state &) const = default;
    };

    class picker_state {
    public:
        // invocation
        bool on_key_control_shift;
        bool on_key_control_alt;
        bool on_key_alt_shift;
        bool on_key_caps;
        bool on_rule_conflict;
        bool on_no_rule;
        bool always;

        // general
        float icon_size;
        float item_padding;
        float inactive_item_alpha;
        bool show_key_hints;
        int border_width;
        bool show_native_chrome;
        int opacity;
        bool close_on_focus_loss;
        bool always_on_top;

        bool operator==(const picker_state &) const = default;
    };

    class substitition_state {
    public:
        std::string kind;
        std::string find;
        std::string replace;

        bool operator==(const substitition_state &) const = default;
    };

    class transforms_state {
    public:
        bool unwrap_o365;
        bool unshorten;
        bool substitute;
        bool scripting;
        std::vector<substitition_state> substitutions;

        bool operator==(const transforms_state &) const = default;
    };

    class state : public grey::common::app_state {
    public:
        using node = fkyaml::node;
        using string = std::string;
        using strings = std::vector<std::string>;

        string ui_theme;
        bool log_rule_hits;
        bool show_hidden_browsers;
        icon_overlay_mode icon_overlay;
        bool discover_classic_gecko_profiles;
        bool discover_gecko_containers;
        toast_state toast;
        picker_state picker;
        transforms_state transforms;
        string pv_last_url;
        string pv_last_wt;
        string pv_last_pn;
        std::vector<std::shared_ptr<bt::browser>> browsers;

        bool operator==(const state &other) const {
            return ui_theme == other.ui_theme &&
                   log_rule_hits == other.log_rule_hits &&
                   show_hidden_browsers == other.show_hidden_browsers &&
                   icon_overlay == other.icon_overlay &&
                   discover_classic_gecko_profiles == other.discover_classic_gecko_profiles &&
                   discover_gecko_containers == other.discover_gecko_containers &&
                   toast == other.toast &&
                   picker == other.picker &&
                   transforms == other.transforms &&
                   browsers == other.browsers;
        }

        bool operator!=(const state &other) const {
            return !(*this == other);
        }

        state(const std::string &application_name) : h{application_name} {
            state::deserialize();
        }

        ~state() override {
            state::serialize();
        }

        void deserialize() override {
            h.deserialize();

            string tmp;
            h.read<string>(h.root, "ui_theme", ui_theme, "");
            h.read<bool>(h.root, "log_rule_hits", log_rule_hits, false);
            h.read<bool>(h.root, "show_hidden_browsers", show_hidden_browsers, false);
            h.read<string>(h.root, "icon_overlay", tmp, "");
            icon_overlay = magic_enum::enum_cast<icon_overlay_mode>(tmp, magic_enum::case_insensitive).value_or(
                icon_overlay_mode::profile_on_browser);
            h.read<bool>(h.root, "discover_classic_gecko_profiles", discover_classic_gecko_profiles, false);
            h.read<bool>(h.root, "discover_gecko_containers", discover_gecko_containers, true);

            // toast
            node& toast_node = mp(h.root, "toast");
            h.read<bool>(toast_node, "enabled", toast.enabled, true);
            h.read<int>(toast_node, "visible_seconds", toast.visible_seconds, 3);
            h.read<int>(toast_node, "border_width", toast.border_width, 1);

            // picker
            node& picker_node = mp(h.root, "picker");
            node& picker_invoke_node = mp(picker_node, "invoke");
            h.read<bool>(picker_invoke_node, "on_key_control_shift", picker.on_key_control_shift, false);
            h.read<bool>(picker_invoke_node, "on_key_control_alt", picker.on_key_control_alt, false);
            h.read<bool>(picker_invoke_node, "on_key_alt_shift", picker.on_key_alt_shift, false);
            h.read<bool>(picker_invoke_node, "on_key_caps", picker.on_key_caps, false);
            h.read<bool>(picker_invoke_node, "on_rule_conflict", picker.on_rule_conflict, false);
            h.read<bool>(picker_invoke_node, "on_no_rule", picker.on_no_rule, false);
            h.read<bool>(picker_invoke_node, "always", picker.always, false);
            h.read<float>(picker_node, "icon_size", picker.icon_size, 32.0f);
            h.read<float>(picker_node, "item_padding", picker.item_padding,  10.0f);
            h.read<float>(picker_node, "inactive_icon_alpha", picker.inactive_item_alpha, 0.4f);
            h.read<bool>(picker_node, "show_key_hints", picker.show_key_hints, true);
            h.read<int>(picker_node, "border_width", picker.border_width, 1);
            h.read<bool>(picker_node, "show_native_chrome", picker.show_native_chrome, false);
            h.read<int>(picker_node, "opacity", picker.opacity, 255);
            h.read<bool>(picker_node, "close_on_focus_loss", picker.close_on_focus_loss, false);
            h.read<bool>(picker_node, "always_on_top", picker.always_on_top, false);

            node& pipevis_node = mp(h.root, "pipevis");
            h.read<string>(pipevis_node, "last_url", pv_last_url, "");
            h.read<string>(pipevis_node, "last_wt", pv_last_wt, "");
            h.read<string>(pipevis_node, "last_pn", pv_last_pn, "");

            node& transforms_node = mp(h.root, "transforms");
            h.read<bool>(transforms_node, "unwrap_o365", transforms.unwrap_o365, true);
            h.read<bool>(transforms_node, "unshorten", transforms.unshorten, true);
            h.read<bool>(transforms_node, "substitute", transforms.substitute, true);
            h.read<bool>(transforms_node, "scripting", transforms.scripting, true);
            // read sequence of substitutions
            transforms.substitutions.clear();
            auto seq = transforms_node["substitutions"];
            if(seq.is_sequence()) {
                for(auto& item_node : seq) {
                    substitition_state item;
                    h.read<string>(item_node, "kind", item.kind, "");
                    h.read<string>(item_node, "find", item.find, "");
                    h.read<string>(item_node, "replace", item.replace, "");
                    transforms.substitutions.push_back(item);
                }
            }

            // browsers
            browsers.clear();
            if(h.root.contains("browsers") && h.root["browsers"].is_sequence()) {
                for(auto& bnode : h.root["browsers"]) {
                    string name, cmd;
                    h.read<string>(bnode, "name", name, "");
                    h.read<string>(bnode, "cmd", cmd, "");
                    auto b = std::make_shared<bt::browser>(name, cmd);
                    h.read<bool>(bnode, "visible", b->is_hidden, true);
                    b->is_hidden = !b->is_hidden;
                    h.read<string>(bnode, "icon", b->icon_path, "");
                    h.read<string>(bnode, "data", b->data_path, "");
                    string engine_str;
                    h.read<string>(bnode, "engine", engine_str, "");
                    b->engine = magic_enum::enum_cast<browser_engine>(engine_str, magic_enum::case_insensitive).value_or(browser_engine::generic);
                    h.read<bool>(bnode, "autodiscovered", b->is_autodiscovered, false);

                    if(bnode.contains("profiles") && bnode["profiles"].is_sequence()) {
                        for(auto& pnode : bnode["profiles"]) {
                            string name, launch_arg, icon;
                            h.read<string>(pnode, "name", name, "");
                            h.read<string>(pnode, "arg", launch_arg, "");
                            h.read<string>(pnode, "icon", icon, "");

                            auto p = std::make_shared<bt::browser_instance>(b, name, launch_arg, icon);

                            h.read<string>(pnode, "user_arg", p->user_arg, "");
                            h.read<string>(pnode, "user_icon", p->user_icon_path, "");
                            h.read<bool>(pnode, "incognito", p->is_incognito, false);
                            h.read<bool>(pnode, "visible", p->is_hidden, true);
                            p->is_hidden = !p->is_hidden;

                            if(pnode.contains("rules") && pnode["rules"].is_sequence()) {
                                for(auto& rnode : pnode["rules"]) {
                                    auto r = std::make_shared<match_rule>();
                                    h.read<string>(rnode, "value", r->value, "");
                                    h.read<string>(rnode, "loc", tmp, "");
                                    r->loc = magic_enum::enum_cast<match_location>(tmp, magic_enum::case_insensitive).value_or(match_location::url);
                                    h.read<string>(rnode, "scope", tmp, "");
                                    r->scope = magic_enum::enum_cast<match_scope>(tmp, magic_enum::case_insensitive).value_or(match_scope::any);
                                    h.read<bool>(rnode, "is_regex", r->is_regex, false);
                                    h.read<bool>(rnode, "app_mode", r->app_mode, false);
                                    h.read<bool>(rnode, "is_fallback", r->is_fallback, false);
                                    p->rules.push_back(r);
                                }
                            }
                            b->instances.push_back(p);
                        }
                    }
                    browsers.push_back(b);
                }
            }
        }

        void serialize() override {
            h.write(h.root, "ui_theme", ui_theme);
            h.write(h.root, "log_rule_hits", log_rule_hits);
            h.write(h.root, "show_hidden_browsers", show_hidden_browsers);
            h.write(h.root, "icon_overlay", magic_enum::enum_name(icon_overlay));
            h.write(h.root, "discover_classic_gecko_profiles", discover_classic_gecko_profiles);
            h.write(h.root, "discover_gecko_containers", discover_gecko_containers);

            node& toast_node = mp(h.root, "toast");
            h.write(toast_node, "enabled", toast.enabled);
            h.write(toast_node, "visible_seconds", toast.visible_seconds);
            h.write(toast_node, "border_width", toast.border_width);

            node& picker_node = mp(h.root, "picker");
            node& picker_invoke_node = mp(picker_node, "invoke");
            h.write(picker_invoke_node, "on_key_control_shift", picker.on_key_control_shift);
            h.write(picker_invoke_node, "on_key_control_alt", picker.on_key_control_alt);
            h.write(picker_invoke_node, "on_key_alt_shift", picker.on_key_alt_shift);
            h.write(picker_invoke_node, "on_key_caps", picker.on_key_caps);
            h.write(picker_invoke_node, "on_rule_conflict", picker.on_rule_conflict);
            h.write(picker_invoke_node, "on_no_rule", picker.on_no_rule);
            h.write(picker_invoke_node, "always", picker.always);
            h.write(picker_node, "icon_size", picker.icon_size);
            h.write(picker_node, "item_padding", picker.item_padding);
            h.write(picker_node, "inactive_icon_alpha", picker.inactive_item_alpha);
            h.write(picker_node, "show_key_hints", picker.show_key_hints);
            h.write(picker_node, "border_width", picker.border_width);
            h.write(picker_node, "show_native_chrome", picker.show_native_chrome);
            h.write(picker_node, "opacity", picker.opacity);
            h.write(picker_node, "close_on_focus_loss", picker.close_on_focus_loss);
            h.write(picker_node, "always_on_top", picker.always_on_top);

            node& pipevis_node = mp(h.root, "pipevis");
            h.write(pipevis_node, "last_url", pv_last_url);
            h.write(pipevis_node, "last_wt", pv_last_wt);
            h.write(pipevis_node, "last_pn", pv_last_pn);

            node& transforms_node = mp(h.root, "transforms");
            h.write(transforms_node, "unwrap_o365", transforms.unwrap_o365);
            h.write(transforms_node, "unshorten", transforms.unshorten);
            h.write(transforms_node, "substitute", transforms.substitute);
            h.write(transforms_node, "scripting", transforms.scripting);
            transforms_node["substitutions"] = node::sequence();
            auto& seq = transforms_node["substitutions"].get_value_ref<node::sequence_type&>();
            for(auto& item : transforms.substitutions) {
                node item_node = node::mapping();
                h.write(item_node, "kind", item.kind);
                h.write(item_node, "find", item.find);
                h.write(item_node, "replace", item.replace);
                seq.push_back(item_node);
            }

            // browsers
            h.root["browsers"] = node::sequence();
            auto& bseq = h.root["browsers"].get_value_ref<node::sequence_type&>();
            for(auto& b : browsers) {
                node bnode = node::mapping();
                h.write(bnode, "name", b->name);
                h.write(bnode, "cmd", b->open_cmd);
                h.write(bnode, "visible", !b->is_hidden);
                if(!b->icon_path.empty())
                    h.write(bnode, "icon", b->icon_path);
                if(!b->data_path.empty())
                    h.write(bnode, "data", b->data_path);
                if(b->engine != browser_engine::generic)
                    h.write(bnode, "engine", magic_enum::enum_name(b->engine));
                h.write(bnode, "autodiscovered", b->is_autodiscovered);

                // profiles
                bnode["profiles"] = node::sequence();
                auto& pseq = bnode["profiles"].get_value_ref<node::sequence_type&>();
                for(auto p : b->instances) {
                    node pnode = node::mapping();
                    h.write(pnode, "name", p->name);
                    h.write(pnode, "arg", p->launch_arg);
                    if(!p->user_arg.empty())
                        h.write(pnode, "user_arg", p->user_arg);
                    if(!p->icon_path.empty())
                        h.write(pnode, "icon", p->icon_path);
                    if(!p->user_icon_path.empty())
                        h.write(pnode, "user_icon", p->user_icon_path);
                    h.write(pnode, "incognito", p->is_incognito);
                    h.write(pnode, "visible", !p->is_hidden);

                    // rules
                    if(!p->rules.empty()) {
                        pnode["rules"] = node::sequence();
                        auto& rseq = pnode["rules"].get_value_ref<node::sequence_type&>();
                        for(auto& r : p->rules) {
                            node rnode = node::mapping();
                            h.write(rnode, "value", r->value);
                            h.write(rnode, "loc", magic_enum::enum_name(r->loc));
                            h.write(rnode, "scope", magic_enum::enum_name(r->scope));
                            h.write(rnode, "is_regex", r->is_regex);
                            h.write(rnode, "app_mode", r->app_mode);
                            h.write(rnode, "is_fallback", r->is_fallback);
                            rseq.push_back(rnode);
                        }
                    }

                    pseq.push_back(pnode);
                }

                bseq.push_back(bnode);
            }

            h.serialize();
        }

        std::filesystem::file_time_type get_last_write_time() const override {
            return h.get_last_write_time();
        }

    private:
        grey::common::state_handler h;
        float last_flushed_ago{0.f};

        static node &mp(node &parent, const std::string &key) {
            if(!parent.contains(key)) {
                parent[key] = fkyaml::node::mapping();
            }

            return parent[key];
        }
    };
}
