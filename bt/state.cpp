#include "state.h"
using namespace std;

namespace bt {

    template <typename T>
    T read(const fkyaml::node& node, const std::string& key, T default_value) {
        if (!node.contains(key)) {
            return default_value;
        }
        const auto& child = node[key];
        try {
            return child.get_value<T>();
        } catch (const fkyaml::exception&) {
            return default_value; // wrong type, or conversion failed
        }
    }

    void to_node(fkyaml::node& node, const toast_state& state) {
        node = fkyaml::node{
                        {"enabled", state.enabled},
                        {"visible_seconds", state.visible_seconds},
                        {"border_width", state.border_width}
        };
    }

    void from_node(const fkyaml::node& node, toast_state& state) {
        state.enabled = read<bool>(node, "enabled", true);
        state.visible_seconds = read<int>(node, "visible_seconds", 5);
        state.border_width = read<int>(node, "border_width", 1);
    }

    void to_node(fkyaml::node &node, const picker_state &state) {
        node = fkyaml::node{
            {"on_key_control_shift", state.on_key_control_shift},
            {"on_key_control_alt", state.on_key_control_alt},
            {"on_key_alt_shift", state.on_key_alt_shift},
            {"on_key_caps", state.on_key_caps},
            {"on_rule_conflict", state.on_rule_conflict},
            {"on_no_rule", state.on_no_rule},
            {"always", state.always},

            {"icon_size", state.icon_size},
            {"item_padding", state.item_padding},
            {"inactive_icon_alpha", state.inactive_item_alpha},
            {"show_key_hints", state.show_key_hints},
            {"border_width", state.border_width},
            {"show_native_chrome", state.show_native_chrome},
            {"opacity", state.opacity},
            {"close_on_focus_loss", state.close_on_focus_loss},
            {"always_on_top", state.always_on_top}
        };
    }

    void from_node(const fkyaml::node &node, picker_state &state) {
        state.on_key_control_shift = read<bool>(node, "on_key_control_shift", false);
        state.on_key_control_alt = read<bool>(node, "on_key_control_alt", false);
        state.on_key_alt_shift = read<bool>(node, "on_key_alt_shift", false);
        state.on_key_caps = read<bool>(node, "on_key_caps", false);
        state.on_rule_conflict = read<bool>(node, "on_rule_conflict", false);
        state.on_no_rule = read<bool>(node, "on_no_rule", false);
        state.always = read<bool>(node, "always", false);
        state.icon_size = read<float>(node, "icon_size", 32.0f);
        state.item_padding = read<float>(node, "item_padding", 10.0f);
        state.inactive_item_alpha = read<float>(node, "inactive_icon_alpha", 0.4f);
        state.show_key_hints = read<bool>(node, "show_key_hints", true);
        state.border_width = read<int>(node, "border_width", 1);
        state.show_native_chrome = read<bool>(node, "show_native_chrome", false);
        state.opacity = read<int>(node, "opacity", 255);
        state.close_on_focus_loss = read<bool>(node, "close_on_focus_loss", false);
        state.always_on_top = read<bool>(node, "always_on_top", false);
    }

    void to_node(fkyaml::node &node, const substitition_state &state) {
        node = {
            {"kind", state.kind},
            {"find", state.find},
            {"replace", state.replace}
        };
    }

    void from_node(const fkyaml::node &node, substitition_state &state) {
        state.kind = read<string>(node, "kind", "");
        state.find = read<string>(node, "find", "");
        state.replace = read<string>(node, "replace", "");
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
        state.scripting = read<bool>(node, "scripting", true);
        state.substitute = read<bool>(node, "substitute", true);
        state.unshorten = read<bool>(node, "unshorten", true);
        state.unwrap_o365 = read<bool>(node, "unwrap_o365", true);
        state.substitutions = read<std::vector<substitition_state>>(node, "substitutions", std::vector<substitition_state>{});
    }

    void to_node(fkyaml::node &node, const pipevis_state &state) {
        node = {
            {"url", state.url},
            {"window_title", state.window_title},
            {"process_name", state.process_name}
        };
    }

    void from_node(const fkyaml::node &node, pipevis_state &state) {
        state.url = read<string>(node, "url", "");
        state.window_title = read<string>(node, "window_title", "");
        state.process_name = read<string>(node, "process_name", "");
    }

    void from_node(const fkyaml::node &node, state &state) {
        state.ui_theme = read<string>(node, "ui_theme", "");
        state.log_rule_hits = read<bool>(node, "log_rule_hits", false);
        state.show_hidden_browsers = read<bool>(node, "show_hidden_browsers", false);
        
        string tmp = read<string>(node, "icon_overlay", "");
        state.icon_overlay = magic_enum::enum_cast<icon_overlay_mode>(tmp, magic_enum::case_insensitive).value_or(icon_overlay_mode::profile_on_browser);

        state.discover_classic_gecko_profiles = read<bool>(node, "discover_classic_gecko_profiles", false);
        state.discover_gecko_containers = read<bool>(node, "discover_gecko_containers", true);

        state.toast = read<toast_state>(node, "toast", toast_state{});
        state.picker = read<picker_state>(node, "picker", picker_state{});
        state.transforms = read<transforms_state>(node, "transforms", transforms_state{});
        state.pipevis = read<pipevis_state>(node, "pipevis", pipevis_state{});

        // browser does not have a default constructor
        //state.browsers = read<std::vector<browser>>(node, "browsers", std::vector<browser>{});
        state.browsers.clear();
        if(node.contains("browsers") && node["browsers"].is_sequence()) {
            for(auto& bnode : node["browsers"]) {
                string name = read<string>(bnode, "name", "");
                string cmd = read<string>(bnode, "cmd", "");
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

    void from_node(const fkyaml::node& node, match_rule& state) {
        state.value = read<string>(node, "value", "");
        state.loc = magic_enum::enum_cast<match_location>(
            read<string>(node, "loc", ""), magic_enum::case_insensitive)
            .value_or(match_location::url);
        state.scope = magic_enum::enum_cast<match_scope>(read<string>(node, "scope", ""), magic_enum::case_insensitive)
            .value_or(match_scope::any);
        state.is_regex = read<bool>(node, "is_regex", false);
        state.app_mode = read<bool>(node, "app_mode", false);
    }

    void to_node(fkyaml::node& node, const match_rule& state) {
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

    void from_node(const fkyaml::node& node, browser_profile& state) {
        state.user_arg = read<string>(node, "user_arg", "");
        state.user_icon_path = read<string>(node, "user_icon", "");
        state.is_incognito = read<bool>(node, "incognito", false);
        state.is_hidden = !read<bool>(node, "visible", true);
        state.rules = read<vector<match_rule>>(node, "rules", vector<match_rule>{});
    }

    void to_node(fkyaml::node& node, const browser_profile& state) {
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
        state.name = read<std::string>(node, "name", "");
        state.open_cmd = read<std::string>(node, "cmd", "");
        state.is_hidden = !read<bool>(node, "visible", true);
        state.icon_path = read<std::string>(node, "icon", "");
        state.data_path = read<std::string>(node, "data", "");
        state.engine = magic_enum::enum_cast<browser_engine>(read<std::string>(node, "engine", "")).value_or(browser_engine::generic);

        // profiles do not have a default constructor
        //state.profiles = read<std::vector<browser_profile>>(node, "profiles", {});
        state.profiles.clear();
        if(node.contains("profiles") && node["profiles"].is_sequence()) {
            for(const fkyaml::node& bnode : node["profiles"]) {
                string name = read<string>(bnode, "name", "");
                string launch_arg = read<string>(bnode, "arg", "");
                string icon_path = read<string>(bnode, "icon", "");
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
