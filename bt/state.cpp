#include "state.h"
#include "magic_enum/magic_enum.hpp"

using namespace std;
using namespace grey::common;

namespace bt {
    void to_node(fkyaml::node &node, const toast_state &state) {
        node = fkyaml::node{
            {"enabled", state.enabled},
            {"visible_seconds", state.visible_seconds},
            {"border_width", state.border_width}
        };
    }

    void from_node(const fkyaml::node &node, toast_state &state) {
        read<bool>(node, "enabled", state.enabled);
        read<int>(node, "visible_seconds", state.visible_seconds);
        read<int>(node, "border_width", state.border_width);
    }

    void to_node(fkyaml::node &node, const picker_invoke_state &state) {
        node = fkyaml::node{
            {"on_key_control_shift", state.on_key_control_shift},
            {"on_key_control_alt", state.on_key_control_alt},
            {"on_key_alt_shift", state.on_key_alt_shift},
            {"on_key_caps_locks", state.on_key_caps_locks},
            {"on_no_rule", state.on_no_rule}
        };
    }

    void from_node(const fkyaml::node &node, picker_invoke_state &state) {
        read<bool>(node, "on_key_control_shift", state.on_key_control_shift);
        read<bool>(node, "on_key_control_alt", state.on_key_control_alt);
        read<bool>(node, "on_key_alt_shift", state.on_key_alt_shift);
        read<bool>(node, "on_key_caps_locks", state.on_key_caps_locks);
        read<bool>(node, "on_no_rule", state.on_no_rule);
    }

    void to_node(fkyaml::node &node, const picker_state &state) {
        node = fkyaml::node{
            {"icon_size", state.icon_size},
            {"item_padding", state.item_padding},
            {"inactive_icon_alpha", state.inactive_item_alpha},
            {"show_key_hints", state.show_key_hints},
            {"border_width", state.border_width},
            {"show_native_chrome", state.show_native_chrome},
            {"opacity", state.opacity},
            {"close_on_focus_loss", state.close_on_focus_loss},
            {"always_on_top", state.always_on_top},
            {"invoke", state.invoke}
        };
    }

    void from_node(const fkyaml::node &node, picker_state &state) {
        read<float>(node, "icon_size", state.icon_size);
        read<float>(node, "item_padding", state.item_padding);
        read<float>(node, "inactive_icon_alpha", state.inactive_item_alpha);
        read<bool>(node, "show_key_hints", state.show_key_hints);
        read<int>(node, "border_width", state.border_width);
        read<bool>(node, "show_native_chrome", state.show_native_chrome);
        read<int>(node, "opacity", state.opacity);
        read<bool>(node, "close_on_focus_loss", state.close_on_focus_loss);
        read<bool>(node, "always_on_top", state.always_on_top);
        read<picker_invoke_state>(node, "invoke", state.invoke);
    }

    void to_node(fkyaml::node &node, const substitition_state &state) {
        node = {
            {"kind", state.kind},
            {"find", state.find},
            {"replace", state.replace}
        };
    }

    void from_node(const fkyaml::node &node, substitition_state &state) {
        read<string>(node, "kind", state.kind);
        read<string>(node, "find", state.find);
        read<string>(node, "replace", state.replace);
    }

    void to_node(fkyaml::node &node, const transforms_state &state) {
        node = {
            {"scripting", state.scripting},
            {"substitute", state.substitute},
            {"unshorten", state.unshorten},
            {"unwrap_o365", state.unwrap_o365}
        };
        if(!state.substitutions.empty())
            node["substitutions"] = state.substitutions;
    }

    void from_node(const fkyaml::node &node, transforms_state &state) {
        read<bool>(node, "scripting", state.scripting);
        read<bool>(node, "substitute", state.substitute);
        read<bool>(node, "unshorten", state.unshorten);
        read<bool>(node, "unwrap_o365", state.unwrap_o365);
        read<std::vector<substitition_state> >(node, "substitutions", state.substitutions);
    }

    void to_node(fkyaml::node &node, const pipevis_state &state) {
        node = {
            {"url", state.url},
            {"window_title", state.window_title},
            {"process_name", state.process_name}
        };
    }

    void from_node(const fkyaml::node &node, pipevis_state &state) {
        read<string>(node, "url", state.url);
        read<string>(node, "window_title", state.window_title);
        read<string>(node, "process_name", state.process_name);
    }

    void from_node(const fkyaml::node &node, state &state) {
        read<string>(node, "ui_theme", state.ui_theme);
        read<bool>(node, "log_rule_hits", state.log_rule_hits);
        read<bool>(node, "show_hidden_browsers", state.show_hidden_browsers);

        string tmp;
        read<string>(node, "icon_overlay", tmp);
        state.icon_overlay = magic_enum::enum_cast<icon_overlay_mode>(tmp, magic_enum::case_insensitive).value_or(
            icon_overlay_mode::profile_on_browser);

        read<bool>(node, "discover_classic_gecko_profiles", state.discover_classic_gecko_profiles);
        read<bool>(node, "discover_gecko_containers", state.discover_gecko_containers);

        read<toast_state>(node, "toast", state.toast);
        read<picker_state>(node, "picker", state.picker);
        read<transforms_state>(node, "transforms", state.transforms);
        read<pipevis_state>(node, "pipevis", state.pipevis);

        // browser does not have a default constructor
        //state.browsers = read<std::vector<browser>>(node, "browsers", std::vector<browser>{});
        state.browsers.clear();
        if(node.contains("browsers") && node["browsers"].is_sequence()) {
            for(auto &bnode: node["browsers"]) {
                string name, cmd;
                read<string>(bnode, "name", name);
                read<string>(bnode, "cmd", cmd);
                browser b{name, cmd};
                from_node(bnode, b);
                state.browsers.push_back(b);
            }
        }
    }

    void to_node(fkyaml::node &node, const state &state) {
        node = fkyaml::node::mapping();
        node["ui_theme"] = state.ui_theme;
        node["log_rule_hits"] = state.log_rule_hits;
        node["show_hidden_browsers"] = state.show_hidden_browsers;
        node["icon_overlay"] = magic_enum::enum_name(state.icon_overlay);
        node["discover_classic_gecko_profiles"] = state.discover_classic_gecko_profiles;
        node["discover_gecko_containers"] = state.discover_gecko_containers;
        node["toast"] = state.toast;
        node["picker"] = state.picker;
        node["transforms"] = state.transforms;
        node["pipevis"] = state.pipevis;
        node["browsers"] = state.browsers;
    }

    void from_node(const fkyaml::node &node, match_rule &state) {
        read<string>(node, "value", state.value);
        string tmp;
        if(read<string>(node, "loc", tmp)) {
            state.loc = magic_enum::enum_cast<match_location>(
                        tmp, magic_enum::case_insensitive)
                    .value_or(match_location::url);
        }
        if(read<string>(node, "scope", tmp)) {
            state.scope = magic_enum::enum_cast<match_scope>(tmp, magic_enum::case_insensitive)
                    .value_or(match_scope::any);
        }
        read<bool>(node, "is_regex", state.is_regex);
        read<bool>(node, "app_mode", state.app_mode);
    }

    void to_node(fkyaml::node &node, const match_rule &state) {
        node = {
            {"value", state.value},
            {"loc", magic_enum::enum_name(state.loc)},
        };

        if(state.scope != match_scope::any)
            node["scope"] = magic_enum::enum_name(state.scope);
        if(state.is_regex)
            node["is_regex"] = true;
        if(state.app_mode)
            node["app_mode"] = true;
    }

    void from_node(const fkyaml::node &node, browser_profile &state) {
        read<string>(node, "user_arg", state.user_arg);
        read<string>(node, "user_icon", state.user_icon_path);
        read<bool>(node, "incognito", state.is_incognito);
        if(read<bool>(node, "visible", state.is_hidden)) state.is_hidden = !state.is_hidden;
        read<vector<match_rule> >(node, "rules", state.rules);
    }

    void to_node(fkyaml::node &node, const browser_profile &state) {
        node = {
            {"name", state.name},
            {"arg", state.launch_arg}
        };
        if(!state.user_arg.empty())
            node["user_arg"] = state.user_arg;
        if(!state.icon_path.empty())
            node["icon"] = state.icon_path;
        if(!state.user_icon_path.empty())
            node["user_icon"] = state.user_icon_path;
        if(state.is_incognito)
            node["incognito"] = true;
        if(state.is_hidden)
            node["visible"] = false;
        if(!state.rules.empty())
            node["rules"] = state.rules;
    }

    void from_node(const fkyaml::node &node, browser &state) {
        read<std::string>(node, "name", state.name);
        read<std::string>(node, "cmd", state.open_cmd);
        if(read<bool>(node, "visible", state.is_hidden)) state.is_hidden = !state.is_hidden;
        read<std::string>(node, "icon", state.icon_path);
        read<std::string>(node, "data", state.data_path);
        string tmp;
        if(read<string>(node, "engine", tmp)) {
            state.engine = magic_enum::enum_cast<browser_engine>(tmp).value_or(
                browser_engine::generic);
        }

        // profiles do not have a default constructor
        //state.profiles = read<std::vector<browser_profile>>(node, "profiles", {});
        state.profiles.clear();
        if(node.contains("profiles") && node["profiles"].is_sequence()) {
            for(const fkyaml::node &bnode: node["profiles"]) {
                string name;
                string launch_arg;
                string icon_path;
                read<string>(bnode, "name", name);
                read<string>(bnode, "arg", launch_arg);
                read<string>(bnode, "icon", icon_path);
                browser_profile profile{name, launch_arg, icon_path};
                from_node(bnode, profile);
                state.profiles.push_back(profile);
            }
        }
    }

    void to_node(fkyaml::node &node, const browser &state) {
        node = {
            {"name", state.name},
            {"cmd", state.open_cmd},
            {"engine", magic_enum::enum_name(state.engine)}
        };
        if(state.is_hidden) node["visible"] = false;
        if(!state.icon_path.empty()) node["icon"] = state.icon_path;
        if(!state.data_path.empty()) node["data"] = state.data_path;
        node["profiles"] = state.profiles;
    }
}
