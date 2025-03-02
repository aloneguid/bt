#pragma once
#include <string>
#include <vector>
#include <memory>
#include "match_rule.h"
#include "click_payload.h"

namespace bt {

    class browser_instance;
    class browser_match_result;

    class browser {
    public:

        static const std::string UwpCmdPrefix;

        browser(
            const std::string& id,
            const std::string& name,
            const std::string& open_cmd,
            bool is_system = true);

        std::string id;
        std::string name;
        std::string open_cmd;
        bool is_chromium;
        bool is_firefox;

        /**
         * @brief when true, this browser is part of the system i.e not a user defined one.
        */
        bool is_system;

        /**
         * @brief Whether to hide this browser from UI.
        */
        bool is_hidden{false};

        /**
         * @brief When provided, takes precedence over the default icon extracted from the executable.
         */
        std::string icon_path;

        std::vector<std::shared_ptr<browser_instance>> instances;

        // instance id, non persistent, used in discovery process
        std::string disco_instance_id;

        // UI helper properties
        bool ui_is_hovered{false};
        bool ui_test_url_matches{false};

        size_t get_total_rule_count() const;

        bool get_supports_frameless_windows() const { return supports_frameless_windows; }

        bool is_wellknown() const { return is_chromium || is_firefox; }

        bool is_msstore() const { return open_cmd.starts_with(UwpCmdPrefix); }

        std::string get_best_icon_path() const;

        bool contains_profile_id(const std::string& long_id) const;

        friend bool operator==(const browser& b1, const browser& b2);

        // ---- static members

        static std::vector<std::shared_ptr<browser_instance>> to_instances(
            const std::vector<std::shared_ptr<browser>>& browsers,
            bool skip_hidden = true);

        /**
         * @brief Finds and returns profile by it's long id. If not found, returns the first profile or the first browser.
         * @param browsers 
         * @param long_sys_name 
         * @param found 
         * @return 
         */
        static std::shared_ptr<browser_instance> find_profile_by_long_id(
            const std::vector<std::shared_ptr<browser>>& browsers, const std::string& long_sys_name, bool& found);

        static std::vector<browser_match_result> match(
            const std::vector<std::shared_ptr<browser>>& browsers,
            const click_payload& up,
            const std::string& default_profile_long_id,
            const script_site& script);

        static std::shared_ptr<browser_instance> get_default(
            const std::vector<std::shared_ptr<browser>>& browsers,
            const std::string& default_profile_long_id);

        static std::vector<std::shared_ptr<browser>> merge(
            std::vector<std::shared_ptr<browser>> new_set,
            std::vector<std::shared_ptr<browser>> old_set);

        static size_t index_of(std::vector<std::shared_ptr<bt::browser>>& browsers, std::shared_ptr<bt::browser> b);

    private:

        const bool supports_frameless_windows;

        static std::string get_image_name(const std::string& open_cmd);

        static std::string unmangle_open_cmd(const std::string& open_cmd);
    };

    class browser_instance {
    public:
        // profile:
        // - sys name
        // - display name
        // - icon (HICON)

        inline static const std::string URL_ARG_NAME{ "%url%" };

        browser_instance(
            std::shared_ptr<browser> b,
            const std::string& id,
            const std::string& name,
            const std::string& launch_arg,
            const std::string& icon_path);

        ~browser_instance();

        void launch(click_payload up) const;

        bool is_match(const click_payload& up, match_rule& mr) const;

        bool is_match(const click_payload& up, const script_site& ss, match_rule& mr) const;

        /// <summary>
        /// Adds a rule from text. Does not persist.
        /// </summary>
        /// <param name="rule_text"></param>
        /// <returns>true if rule was added - if duplicate is found it's not added.</returns>
        bool add_rule(const std::string& rule_text);

        void delete_rule(const std::string& rule_text);

        std::shared_ptr<browser> b;     // browser it belongs to

        const std::string id;
        std::string name;               // how it's called by the user

        std::string launch_arg;

        /**
         * @brief user-defined argument, added after launch_arg. Only separate from launch_arg to be able to store independently in the config file.
        */
        std::string user_arg;

        std::vector<std::shared_ptr<match_rule>> rules;

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

        int popularity{ 0 };

        // UI helpers
        bool ui_is_hovered{false};
        bool ui_test_url_matches;

        /**
         * @brief Optional sort order
        */
        int order{0};

        bool is_incognito{false};

        /**
         * @brief If this is the default browser profile in this browser. Doesn't have to do anything with user's default choice in BT.
        */
        bool is_default{false};

        /**
         * @brief Whether this profile has https://github.com/honsiorovskyi/open-url-in-container installed.
        */
        bool has_firefox_ouic_addon{false};

        std::string long_id() const { return b->id + ":" + id; }

        bool is_singular() const; // whether this is a singular instance browser (private mode is not taken into account)

        std::string get_best_display_name() const;

        std::string get_best_icon_path(bool include_override = true) const;

        std::vector<std::string> get_rules_as_text_clean() const;
        void set_rules_from_text(std::vector<std::string> rules_txt);

    private:
        void launch_win32_process_and_foreground(const std::string& cmdline) const;
    };

    struct browser_match_result {
        std::shared_ptr<browser_instance> bi;
        match_rule rule;
    };
}