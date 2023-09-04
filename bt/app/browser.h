#pragma once
#include <string>
#include <vector>
#include <memory>
#include "match_rule.h"
#include "url_payload.h"

namespace bt {

    class browser_instance;
    class browser_match_result;

    class browser {
    public:

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

        std::vector<std::shared_ptr<browser_instance>> instances;

        size_t get_total_rule_count() const;

        bool get_supports_frameless_windows() const { return supports_frameless_windows; }

        friend bool operator==(const browser& b1, const browser& b2);

        // ---- static members

        static std::vector<std::shared_ptr<browser>> get_cache(bool reload = false);
        static void persist_cache();

        static std::vector<std::shared_ptr<browser_instance>> to_instances(const std::vector<std::shared_ptr<browser>>& browsers);

        static std::shared_ptr<browser_instance> find_profile_by_long_id(
            const std::vector<std::shared_ptr<browser>>& browsers, const std::string& long_sys_name, bool& found);

        static std::vector<browser_match_result> match(
            const std::vector<std::shared_ptr<browser>>& browsers,
            const std::string& url);

        static std::shared_ptr<browser_instance> get_fallback(const std::vector<std::shared_ptr<browser>>& browsers);

        static std::vector<std::shared_ptr<browser>> merge(
            std::vector<std::shared_ptr<browser>> new_set,
            std::vector<std::shared_ptr<browser>> old_set);

    private:

        static std::vector<std::shared_ptr<browser>> cache;
        const bool supports_frameless_windows;

        static std::string get_image_name(const std::string& open_cmd);

        static bool is_chromium_browser(const std::string& system_id);
        static bool is_firefox_browser(const std::string& system_id);
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

        void launch(url_payload up) const;

        bool is_match(const std::string& url, match_rule& mr) const;

        /// <summary>
        /// Adds a rule from text. Does not persist.
        /// </summary>
        /// <param name="rule_text"></param>
        /// <returns>true if rule was added - if duplicate is found it's not added.</returns>
        bool add_rule(const std::string& rule_text);

        void delete_rule(const std::string& rule_text);

        std::shared_ptr<browser> b;

        const std::string id;
        std::string name;               // how it's called by the user

        std::string launch_arg;

        /**
         * @brief user-defined argument, added after launch_arg. Only separate from launch_arg to be able to store independently in the config file.
        */
        std::string user_arg;

        std::vector<std::shared_ptr<match_rule>> rules;

        /**
         * @brief Optionally sets a custom profile icon if known.
        */
        std::string icon_path;

        int popularity{ 0 };

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

        std::vector<std::string> get_rules_as_text_clean() const;
        void set_rules_from_text(std::vector<std::string> rules_txt);

    private:
    };

    struct browser_match_result {
        std::shared_ptr<browser_instance> bi;
        match_rule rule;
    };
}