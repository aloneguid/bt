#pragma once
#include <string>
#include <vector>
#include "model.h"
#include "common/state_handler.hpp"
#include "common/stl.hpp"
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

    void to_node(fkyaml::node& node, const toast_state& state);

    void from_node(const fkyaml::node& node, toast_state& state);

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

    void to_node(fkyaml::node& node, const picker_state& state);

    void from_node(const fkyaml::node& node, picker_state& state);

    class substitition_state {
    public:
        std::string kind;
        std::string find;
        std::string replace;

        bool operator==(const substitition_state &) const = default;
    };

    void to_node(fkyaml::node& node, const substitition_state& state);

    void from_node(const fkyaml::node& node, substitition_state& state);

    class transforms_state {
    public:
        bool unwrap_o365;
        bool unshorten;
        bool substitute;
        bool scripting;
        std::vector<substitition_state> substitutions;

        bool operator==(const transforms_state &) const = default;
    };

    void to_node(fkyaml::node &node, const transforms_state &state);

    void from_node(const fkyaml::node& node, transforms_state& state);

    class pipevis_state {
    public:
        std::string url;
        std::string window_title;
        std::string process_name;

        bool operator==(const pipevis_state &) const = default;
    };

    void to_node(fkyaml::node &node, const pipevis_state &state);

    void from_node(const fkyaml::node& node, pipevis_state& state);

    class state {
    public:
        std::string ui_theme;
        bool log_rule_hits;
        bool show_hidden_browsers;
        icon_overlay_mode icon_overlay;
        bool discover_classic_gecko_profiles;
        bool discover_gecko_containers;
        toast_state toast;
        picker_state picker;
        transforms_state transforms;
        pipevis_state pipevis;
        std::vector<browser> browsers;

        bool operator==(const state &) const = default;
    };

    void from_node(const fkyaml::node& node, state& state);

    void to_node(fkyaml::node &node, const state &state);

    void from_node(const fkyaml::node& node, browser& state);

    void to_node(fkyaml::node &node, const browser &state);

    class state_container : public grey::common::app_state_container<state> {
    public:
        using node = fkyaml::node;
        using string = std::string;
        using strings = std::vector<std::string>;
        state current_state;

        state_container(const std::string &application_name) : h{application_name} {
            deserialize();
        }

        ~state_container() override {
            serialize();
        }

        bt::state& get_state() override {
            return current_state;
        }

        void deserialize() override {
            h.deserialize();

            from_node(h.root, current_state);

            // browsers
            /*browsers.clear();
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

                    if(bnode.contains("profiles") && bnode["profiles"].is_sequence()) {
                        for(auto& pnode : bnode["profiles"]) {
                            string name, launch_arg, icon;
                            h.read<string>(pnode, "name", name, "");
                            h.read<string>(pnode, "arg", launch_arg, "");
                            h.read<string>(pnode, "icon", icon, "");

                            browser_profile p(b, name, launch_arg, icon);

                            h.read<bool>(pnode, "default", p.is_default, false);
                            h.read<string>(pnode, "user_arg", p.user_arg, "");
                            h.read<string>(pnode, "user_icon", p.user_icon_path, "");
                            h.read<bool>(pnode, "incognito", p.is_incognito, false);
                            h.read<bool>(pnode, "visible", p.is_hidden, true);
                            p.is_hidden = !p.is_hidden;

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
                                    p.rules.push_back(r);
                                }
                            }
                            b->profiles.push_back(p);
                        }
                    }
                    browsers.push_back(b);
                }
            }*/
        }

        void serialize() override {
            to_node(h.root, current_state);

/*
            // browsers
            h.root["browsers"] = node::sequence();
            auto& bseq = h.root["browsers"].get_value_ref<node::sequence_type&>();
            for(auto& b : browsers) {
                node bnode = node::mapping();
                h.write(bnode, "name", b->name);
                h.write(bnode, "cmd", b->open_cmd);
                if(b->is_hidden)
                    h.write(bnode, "visible", !b->is_hidden);
                if(b->is_default())
                    h.write(bnode, "default", true);
                if(!b->icon_path.empty())
                    h.write(bnode, "icon", b->icon_path);
                if(!b->data_path.empty())
                    h.write(bnode, "data", b->data_path);
                if(b->engine != browser_engine::generic)
                    h.write(bnode, "engine", magic_enum::enum_name(b->engine));

                // profiles
                bnode["profiles"] = node::sequence();
                auto& pseq = bnode["profiles"].get_value_ref<node::sequence_type&>();
                for(auto p : b->profiles) {
                    node pnode = node::mapping();
                    h.write(pnode, "name", p.name);
                    h.write(pnode, "arg", p.launch_arg);
                    if(!p.user_arg.empty())
                        h.write(pnode, "user_arg", p.user_arg);
                    if(!p.icon_path.empty())
                        h.write(pnode, "icon", p.icon_path);
                    if(!p.user_icon_path.empty())
                        h.write(pnode, "user_icon", p.user_icon_path);
                    if(p.is_incognito)
                        h.write(pnode, "incognito", p.is_incognito);
                    if(p.is_hidden)
                        h.write(pnode, "visible", !p.is_hidden);

                    // rules
                    if(!p.rules.empty()) {
                        pnode["rules"] = node::sequence();
                        auto& rseq = pnode["rules"].get_value_ref<node::sequence_type&>();
                        for(auto& r : p.rules) {
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
*/
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
