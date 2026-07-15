#pragma once
#include <string>
#include <vector>
#include "model.h"
#include "common/config.hpp"
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

    class picker_invoke_state {
    public:
        bool on_key_control_shift;
        bool on_key_control_alt;
        bool on_key_alt_shift;
        bool on_key_caps;
        bool on_rule_conflict;
        bool on_no_rule;
        bool always;

        bool operator==(const picker_invoke_state &) const = default;
    };

    void to_node(fkyaml::node& node, const picker_invoke_state& state);

    void from_node(const fkyaml::node& node, picker_invoke_state& state);

    class picker_state {
    public:
        float icon_size;
        float item_padding;
        float inactive_item_alpha;
        bool show_key_hints;
        int border_width;
        bool show_native_chrome;
        int opacity;
        bool close_on_focus_loss;
        bool always_on_top;

        picker_invoke_state invoke;

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

    void from_node(const fkyaml::node& node, browser& state);

    void to_node(fkyaml::node &node, const browser &state);

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
}
