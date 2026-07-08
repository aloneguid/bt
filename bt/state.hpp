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

        prop(string, ui_theme);
        prop(bool, show_hidden_browsers);
        prop(bool, toast_enabled);
        prop(int, toast_visible_secs);
        prop(int, toast_border);
        prop(icon_overlay_mode, icon_overlay);
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
            return ui_theme != ui_theme_ ||
                   show_hidden_browsers != show_hidden_browsers_ ||
                   toast_enabled != toast_enabled_ ||
                   toast_visible_secs != toast_visible_secs_ ||
                   toast_border != toast_border_ ||
                   icon_overlay != icon_overlay_ ||
                   discover_firefox_classic_profiles != discover_firefox_classic_profiles_ ||
                   discover_firefox_containers != discover_firefox_containers_;
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

            node &ui = mp(root, "ui");
            node &toast = mp(ui, "toast");
            node &discover = mp(root, "discover");
            node &discover_firefox = mp(discover, "firefox");

            read(ui, string, ui_theme, "theme", "");
            read(ui, bool, show_hidden_browsers, "show_hidden_browsers", false);
            icon_overlay = icon_overlay_ = string_to_icon_overlay_mode(read_string(ui, "icon_overlay", ""));
            read(toast, bool, toast_enabled, "enabled", true);
            read(toast, int, toast_visible_secs, "visible_secs", 3);
            read(toast, int, toast_border, "border", 1);
            read(discover_firefox, bool, discover_firefox_classic_profiles, "classic_profiles", false);
            read(discover_firefox, bool, discover_firefox_containers, "containers", true);
        }

        void serialize() {
            std::ofstream ofs{file_path, std::ios::out};

            node &ui = mp(root, "ui");
            node &toast = mp(ui, "toast");
            node &discover = mp(root, "discover");
            node &discover_firefox = mp(discover, "firefox");

            write(ui, string, ui_theme, "theme");
            write(ui, bool, show_hidden_browsers, "show_hidden_browsers");
            write_string(ui, "icon_overlay", icon_overlay_mode_to_string(icon_overlay_ = icon_overlay));
            write(toast, bool, toast_enabled, "enabled");
            write(toast, int, toast_visible_secs, "visible_secs");
            write(toast, int, toast_border, "border");
            write(discover_firefox, bool, discover_firefox_classic_profiles, "classic_profiles");
            write(discover_firefox, bool, discover_firefox_containers, "containers");

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
