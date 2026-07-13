#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "match_rule.h"
#include "click_payload.h"

namespace bt {
    class browser_profile;
    class browser_match_result;
    class profile_selection;

    enum class browser_engine {
        generic,
        chromium,
        gecko
    };

    class browser {
    public:
        inline static const std::string URL_ARG_NAME{"%url%"};

        browser(
            const std::string &name,
            const std::string &open_cmd);

        bool operator==(const browser &other) const = default;

        std::string name;
        std::string open_cmd;
        browser_engine engine{browser_engine::generic};

        /**
         * @brief Whether to hide this browser from UI.
        */
        bool is_hidden{false};

        /**
         * @brief When provided, takes precedence over the default icon extracted from the executable.
         */
        std::string icon_path;

        /**
         * @brief Location where browser data is stored (e.g. profiles). Depends on the browser.
         */
        std::string data_path;

        std::vector<browser_profile> profiles;

        /**
         * @brief Instance ID, used by Firefox. Not persisted as it's not required after discovery.
         */
        std::string instance_id;

        // UI helper properties
        bool ui_test_url_matches{false};
        float ui_icon_size_anim{0.0f};

        void launch(click_payload up, const browser_profile &profile) const;

        size_t get_total_rule_count() const;

        bool get_supports_frameless_windows() const { return engine == browser_engine::chromium; }

        bool is_wellknown() const { return engine != browser_engine::generic; }

        std::string get_best_icon_path() const;

        std::string get_best_icon_path(const browser_profile &profile, bool include_override = true) const;

        bool is_default() const;

        // ---- static members

        static std::vector<browser_match_result> match(
            const std::vector<browser> &browsers,
            const click_payload &up,
            const script_site &script);

        static std::optional<profile_selection> get_default(const std::vector<browser> &browsers);

        static void set_default(std::vector<browser> &browsers, const browser_profile &profile);

        static std::vector<browser> merge(
            std::vector<browser> &new_set,
            std::vector<browser> &old_set);

        static size_t index_of(std::vector<browser> &browsers, browser &b);

        std::string get_best_display_name(const browser_profile &profile) const;

    private:
        static std::string get_image_name(const std::string &open_cmd);

        void launch_process(const std::string &cmdline, const browser_profile &profile) const;
    };

    class browser_profile {
    public:
        /**
         * @brief Name given by the user
         */
        std::string name;

        std::string launch_arg;

        /**
         * @brief user-defined argument, added after launch_arg. Only separate from launch_arg to be able to store independently in the config file.
        */
        std::string user_arg;

        /**
         * @brief Whether to hide this profile from UI.
        */
        bool is_hidden{false};

        /**
         * @brief Optionally sets a custom profile icon if known.
        */
        std::string icon_path;

        /**
         * @brief User can override the built-in icon with a custom one.
         */
        std::string user_icon_path;

        /**
         * @brief When true, the terminal window will be hidden when launching this profile.
         */
        bool launch_hide_ui{false};

        // UI helpers
        bool ui_test_url_matches;

        bool is_incognito{false};

        /**
         * @brief If this is the default browser profile in this browser. Doesn't have to do anything with user's default choice in BT.
        */
        bool is_default{false};

        /**
         * @brief Whether this profile has https://github.com/honsiorovskyi/open-url-in-container installed.
        */
        bool has_firefox_ouic_addon{false};

        std::vector<match_rule> rules;


        browser_profile(
            const std::string &name,
            const std::string &launch_arg,
            const std::string &icon_path);

        bool operator==(const browser_profile &other) const;

        browser_profile &operator=(const browser_profile &other) {
            name = other.name;
            launch_arg = other.launch_arg;
            user_arg = other.user_arg;
            rules = other.rules;
            icon_path = other.icon_path;
            is_hidden = other.is_hidden;
            user_icon_path = other.user_icon_path;
            launch_hide_ui = other.launch_hide_ui;
            ui_test_url_matches = other.ui_test_url_matches;
            is_incognito = other.is_incognito;
            is_default = other.is_default;
            has_firefox_ouic_addon = other.has_firefox_ouic_addon;
            return *this;
        }

        ~browser_profile();

        bool is_match(const click_payload &up, match_rule &mr) const;

        bool is_match(const click_payload &up, const script_site &ss, match_rule &mr) const;

        /// <summary>
        /// Adds a rule from text. Does not persist.
        /// </summary>
        /// <param name="rule_text"></param>
        /// <returns>true if rule was added - if duplicate is found it's not added.</returns>
        bool add_rule(const std::string &rule_text);

        void delete_rule(const std::string &rule_text);
    };

    class profile_selection {
    public:
        profile_selection(const browser& browser, size_t profile_idx) : root{browser}, profile_idx{profile_idx} {

        }

        const browser& b() const {
            return root;
        }

        const browser_profile& profile() const {
            return root.profiles[profile_idx];
        }

    private:
        bt::browser root;
        size_t profile_idx;
    };

    class browser_match_result {
    public:
        browser_match_result(const profile_selection& selection, const match_rule &rule)
            : profile{selection}, rule(rule) {
        }

        browser_match_result(const browser_match_result &other) = default;

        profile_selection profile;
        match_rule rule;
    };
}
