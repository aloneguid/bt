#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "browser.h"
#include "config/config.h"

namespace bt {

    struct custom_browser_config {
        std::string long_sys_name;
        std::string name;
        std::string cmd;
        std::string arg;
    };

    enum class firefox_container_mode {
        off = 0,
        bt = 1,
        ouic = 2
    };

    class config {
    public:
        // whether to show hidden browsers in the configuration list
        bool show_hidden_browsers{true};
        std::string theme_id;
        bool log_rule_hits{true};
        bool log_app{false};
        firefox_container_mode firefox_mode{firefox_container_mode::off};
        // default browser long sys name
        std::string default_profile_long_id;

        // picker
        // ctrl + shift
        bool picker_on_key_cs;
        bool picker_on_key_ca;
        bool picker_on_key_as;
        bool picker_on_key_cl; // CAPS LOCKS
        // whether to show picker on conflict (more than one browser/profile match)
        bool picker_on_conflict;
        // whether to show picker if none of the rules match at all
        bool picker_on_no_rule;
        // whether to always show the picker, regardless of other settings above (they are kept to restore old behavior when you un-tick)
        bool picker_always;
        bool picker_close_on_focus_loss;
        bool picker_always_on_top;
        bool picker_show_url;

        // pipeline
        bool pipeline_unwrap_o365;
        bool pipeline_unshorten;
        bool pipeline_substitute;
        bool pipeline_script;
        std::vector<std::string> pipeline_substitutions;

        // pipe visualiser
        std::string pv_last_url;
        std::string pv_last_wt;
        std::string pv_last_pn;

        // browser collection
        std::vector<std::shared_ptr<browser>> browsers;

        std::string get_iid();

        config();
        void commit();

        std::string get_absolute_path();

        // experimental flags
        std::string get_flag(const std::string& name);

        // conversion functions
        static std::string firefox_container_mode_to_string(firefox_container_mode mode);
        static firefox_container_mode to_firefox_container_mode(const std::string& name);

        static std::string get_data_file_path(const std::string& name);

    private:
        ::common::config cfg;

        void ensure_instance_id();
        void migrate();
        void load();

        // --- browser/instance

        void save_browsers(std::vector<std::shared_ptr<browser>> browsers);
        std::vector<std::shared_ptr<browser>> load_browsers();
    };
}