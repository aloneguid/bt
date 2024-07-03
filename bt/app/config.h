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
        firefox_container_mode firefox_mode{firefox_container_mode::off};
        // default browser long sys name
        std::string default_browser;

        // picker
        // ctrl + shift
        bool picker_on_key_cs;
        bool picker_on_key_ca;
        bool picker_on_key_as;
        // whether to show picker on conflict (more than one browser/profile match)
        bool picker_on_conflict;
        // whether to show picker if none of the rules match at all
        bool picker_on_no_rule;
        // whether to always show the picker, regardless of other settings above (they are kept to restore old behavior when you un-tick)
        bool picker_always;

        std::vector<std::shared_ptr<browser>> browsers;

        std::string get_iid();

        config();
        void commit();

        std::vector<std::string> get_pipeline();
        void set_pipeline(const std::vector<std::string>& steps);

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