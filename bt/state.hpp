#pragma once
#include <string>
#include "common/fss.h"
#include <filesystem>
#include <fstream>
#include <fkYAML/node.hpp>
#include <imgui.h>
#include "model.h"

// new experimental centralized state

#define prop(type, name) type name, name##_
#define read(container, type, name, key_name, default_value) name = name##_ = read_##type(container, key_name, default_value)
#define write(container, type, name, key_name) write_##type(container, key_name, name##_ = name)

namespace bt {
    class state {
    public:
        using node = fkyaml::node;
        using string = std::string;

        // Uncategorized
        prop(bool, log_rule_hits);

        // Picker
        prop(bool, picker_on_key_cs);
        prop(bool, picker_on_key_ca);
        prop(bool, picker_on_key_as);
        prop(bool, picker_on_key_cl);
        prop(bool, picker_on_conflict);
        prop(bool, picker_on_no_rule);
        prop(bool, picker_always);
        prop(float, picker_icon_size);
        prop(float, picker_item_padding);
        prop(float, picker_inactive_item_alpha);
        prop(bool, picker_show_key_hints);
        prop(int, picker_border_width);
        prop(bool, picker_show_native_chrome);
        prop(int, picker_opacity);
        prop(bool, picker_close_on_focus_loss);
        prop(bool, picker_always_on_top);


        // UI
        prop(string, ui_theme);
        prop(bool, show_hidden_browsers);
        prop(bool, toast_enabled);
        prop(int, toast_visible_secs);
        prop(int, toast_border);
        prop(icon_overlay_mode, icon_overlay);

        // Discovery
        prop(bool, discover_firefox_classic_profiles);
        prop(bool, discover_firefox_containers);

        state(const std::string &application_name) {
            file_path = (std::filesystem::path{grey::common::fss::get_config_dir(application_name)} / "config.yml").
                    string();

            // create dir if not exists
            const std::filesystem::path abs{file_path};
            std::filesystem::create_directories(abs.parent_path());

            deserialize();
        }

        ~state() {
            serialize();
        }

        bool is_dirty() const {
            return log_rule_hits != log_rule_hits_ ||
                   ui_theme != ui_theme_ ||
                   show_hidden_browsers != show_hidden_browsers_ ||
                   toast_enabled != toast_enabled_ ||
                   toast_visible_secs != toast_visible_secs_ ||
                   toast_border != toast_border_ ||
                   icon_overlay != icon_overlay_ ||
                   discover_firefox_classic_profiles != discover_firefox_classic_profiles_ ||
                   discover_firefox_containers != discover_firefox_containers_ ||
                   picker_on_key_cs != picker_on_key_cs_ ||
                   picker_on_key_ca != picker_on_key_ca_ ||
                   picker_on_key_as != picker_on_key_as_ ||
                   picker_on_key_cl != picker_on_key_cl_ ||
                   picker_on_conflict != picker_on_conflict_ ||
                   picker_on_no_rule != picker_on_no_rule_ ||
                   picker_always != picker_always_ ||
                   picker_icon_size != picker_icon_size_ ||
                   picker_item_padding != picker_item_padding_ ||
                   picker_inactive_item_alpha != picker_inactive_item_alpha_ ||
                   picker_show_key_hints != picker_show_key_hints_ ||
                   picker_border_width != picker_border_width_ ||
                   picker_show_native_chrome != picker_show_native_chrome_ ||
                   picker_opacity != picker_opacity_ ||
                   picker_close_on_focus_loss != picker_close_on_focus_loss_ ||
                   picker_always_on_top != picker_always_on_top_;
        }

        void tick() {
            float delta_time = ImGui::GetIO().DeltaTime;
            last_flushed_ago += delta_time;

            if(last_flushed_ago > 1.f && is_dirty()) {
                serialize();
                last_flushed_ago = 0.f;
            }
        }

        void deserialize() {
            std::ifstream ifs{file_path, std::ios::in};
            root = fkyaml::node::deserialize(ifs);
            if(root.get_type() != fkyaml::node_type::MAPPING)
                root = fkyaml::node::mapping();

            node &picker = mp(root, "picker");
            node &toast = mp(root, "toast");
            node &discover = mp(root, "discover");
            node &discover_firefox = mp(discover, "firefox");

            read(root, bool, log_rule_hits, "log_rule_hits", false);
            read(root, string, ui_theme, "ui_theme", "");
            read(root, bool, show_hidden_browsers, "show_hidden_browsers", false);
            icon_overlay = icon_overlay_ = string_to_icon_overlay_mode(read_string(root, "icon_overlay", ""));
            read(toast, bool, toast_enabled, "enabled", true);
            read(toast, int, toast_visible_secs, "visible_secs", 3);
            read(toast, int, toast_border, "border", 1);
            read(discover_firefox, bool, discover_firefox_classic_profiles, "classic_profiles", false);
            read(discover_firefox, bool, discover_firefox_containers, "containers", true);

            read(picker, bool, picker_on_key_cs, "on_key_cs", true);
            read(picker, bool, picker_on_key_ca, "on_key_ca", false);
            read(picker, bool, picker_on_key_as, "on_key_as", false);
            read(picker, bool, picker_on_key_cl, "on_key_cl", false);
            read(picker, bool, picker_on_conflict, "on_conflict", true);
            read(picker, bool, picker_on_no_rule, "on_no_rule", false);
            read(picker, bool, picker_always, "always", false);
            read(picker, float, picker_icon_size, "icon_size", 32.0f);
            read(picker, float, picker_item_padding, "icon_padding", 10.0f);
            read(picker, float, picker_inactive_item_alpha, "inactive_icon_alpha", 0.4f);
            read(picker, bool, picker_show_key_hints, "show_key_hints", true);
            read(picker, int, picker_border_width, "border_width", 1);
            read(picker, bool, picker_show_native_chrome, "show_native_chrome", false);
            read(picker, int, picker_opacity, "opacity", 255);
            read(picker, bool, picker_close_on_focus_loss, "close_on_focus_loss", false);
            read(picker, bool, picker_always_on_top, "always_on_top", false);
        }

        void serialize() {
            std::ofstream ofs{file_path, std::ios::out};

            node &picker = mp(root, "picker");
            node &toast = mp(root, "toast");
            node &discover = mp(root, "discover");
            node &discover_firefox = mp(discover, "firefox");

            write(root, bool, log_rule_hits, "log_rule_hits");
            write(root, string, ui_theme, "ui_theme");
            write(root, bool, show_hidden_browsers, "show_hidden_browsers");
            write_string(root, "icon_overlay", icon_overlay_mode_to_string(icon_overlay_ = icon_overlay));
            write(toast, bool, toast_enabled, "enabled");
            write(toast, int, toast_visible_secs, "visible_secs");
            write(toast, int, toast_border, "border");
            write(discover_firefox, bool, discover_firefox_classic_profiles, "classic_profiles");
            write(discover_firefox, bool, discover_firefox_containers, "containers");

            write(picker, bool, picker_on_key_cs, "on_key_cs");
            write(picker, bool, picker_on_key_ca, "on_key_ca");
            write(picker, bool, picker_on_key_as, "on_key_as");
            write(picker, bool, picker_on_key_cl, "on_key_cl");
            write(picker, bool, picker_on_conflict, "on_conflict");
            write(picker, bool, picker_on_no_rule, "on_no_rule");
            write(picker, bool, picker_always, "always");
            write(picker, float, picker_icon_size, "icon_size");
            write(picker, float, picker_item_padding, "icon_padding");
            write(picker, float, picker_inactive_item_alpha, "inactive_icon_alpha");
            write(picker, bool, picker_show_key_hints, "show_key_hints");
            write(picker, int, picker_border_width, "border_width");
            write(picker, bool, picker_show_native_chrome, "show_native_chrome");
            write(picker, int, picker_opacity, "opacity");
            write(picker, bool, picker_close_on_focus_loss, "close_on_focus_loss");
            write(picker, bool, picker_always_on_top, "always_on_top");

            ofs << root;
        }

    private:
        std::string file_path;
        fkyaml::node root;
        float last_flushed_ago{0.f};

        node &mp(node &parent, const std::string &key) {
            if(!parent.contains(key)) {
                parent[key] = fkyaml::node::mapping();
            }

            return parent[key];
        }

        string read_string(node &n, const std::string &key, const string &default_value) {
            node &value_node = n[key];
            if(value_node.is_string())
                return value_node.get_value<string>();
            return default_value;
        }

        void write_string(node &n, const string &key, const string &value) {
            n[key] = value;
        }

        bool read_bool(node &n, const std::string &key, bool default_value) {
            node &value_node = n[key];
            if(value_node.is_boolean())
                return value_node.get_value<bool>();
            return default_value;
        }

        void write_bool(node &n, const string &key, bool value) {
            n[key] = value;
        }

        int read_int(node &n, const std::string &key, int default_value) {
            node &value_node = n[key];
            if(value_node.is_integer())
                return value_node.get_value<int>();
            return default_value;
        }

        void write_int(node &n, const string &key, int value) {
            n[key] = value;
        }

        float read_float(node &n, const std::string &key, float default_value) {
            node &value_node = n[key];
            if(value_node.is_float_number())
                return value_node.get_value<float>();
            return default_value;
        }

        void write_float(node &n, const string &key, float value) {
            n[key] = value;
        }

        // custom converters
        string icon_overlay_mode_to_string(icon_overlay_mode mode) {
            switch(mode) {
                case icon_overlay_mode::profile_on_browser:
                    return "profile_on_browser";
                case icon_overlay_mode::browser_on_profile:
                    return "browser_on_profile";
                case icon_overlay_mode::browser_only:
                    return "browser";
                case icon_overlay_mode::profile_only:
                    return "profile";
                default:
                    return "profile_on_browser";
            }
        }

        icon_overlay_mode string_to_icon_overlay_mode(const std::string &name) {
            if(name == "profile_on_browser")
                return icon_overlay_mode::profile_on_browser;
            if(name == "browser_on_profile")
                return icon_overlay_mode::browser_on_profile;
            if(name == "browser")
                return icon_overlay_mode::browser_only;
            if(name == "profile")
                return icon_overlay_mode::profile_only;

            return icon_overlay_mode::profile_on_browser;
        }
    };
}

#undef write
#undef read
#undef prop
