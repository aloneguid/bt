#pragma once
#include <string>
#include <vector>
#include "model.h"
#include "common/config.hpp"
#include "app/browser.h"

namespace bt {
    class toast_state {
    public:
        bool enabled{true};
        int visible_seconds{5};
        int border_width{1};

        bool operator==(const toast_state &) const = default;
    };

    void to_node(fkyaml::node& node, const toast_state& state);

    void from_node(const fkyaml::node& node, toast_state& state);

    class picker_invoke_state {
    public:
        bool on_key_control_shift{false};
        bool on_key_control_alt{false};
        bool on_key_alt_shift{false};
        bool on_key_caps_locks{false};
        bool on_no_rule{false};

        bool operator==(const picker_invoke_state &) const = default;
    };

    void to_node(fkyaml::node& node, const picker_invoke_state& state);

    void from_node(const fkyaml::node& node, picker_invoke_state& state);

    class picker_state {
    public:
        float icon_size{32.0f};
        float item_padding{10.0f};
        float inactive_item_alpha{0.4f};
        bool show_key_hints{true};
        int border_width{1};
        bool show_native_chrome{false};
        int opacity{255};
        bool close_on_focus_loss{true};
        bool always_on_top{false};

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
        bool unwrap_o365{true};
        bool unshorten{true};
        bool substitute{true};
        bool scripting{true};
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

    void from_node(const fkyaml::node& node, browser_profile& state);

    void to_node(fkyaml::node &node, const browser_profile &state);

    void from_node(const fkyaml::node& node, browser& state);

    void to_node(fkyaml::node &node, const browser &state);

    class state {
    public:
        std::string ui_theme;
        bool log_rule_hits{false};
        bool show_hidden_browsers{false};
        icon_overlay_mode icon_overlay{icon_overlay_mode::profile_on_browser};
        bool discover_classic_gecko_profiles{false};
        bool discover_gecko_containers{true};
        int highlight_width{4};
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
