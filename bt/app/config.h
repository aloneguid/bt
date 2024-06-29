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
        bool show_hidden_browsers{true};
        std::string theme_id;
        bool log_rule_hits{true};
        firefox_container_mode firefox_mode{firefox_container_mode::off};

        // legacy picker settings
        bool picker_enabled{true};
        std::string open_method;

        // picker
        // when set, picker can be opened manually when holding a hot key and clicking
        std::string picker_hotkey;
        // whether to show picker on conflict (more than one browser/profile match)
        bool picker_on_conflict;
        // whether to show picker if none of the rules match at all
        bool picker_on_no_rule;
        // whether to always show the picker, regardless of other settings above (they are kept to restore old behavior when you un-tick)
        bool picker_always;

        std::string get_iid();

        config();
        void commit();

        void set_fallback(const std::string& long_sys_name);
        std::string get_fallback_long_sys_name();

        void set_persist_popularity(bool v);
        bool get_persist_popularity();
        int get_popularity(const std::string& long_sys_name);
        void set_popularity(const std::string& long_sys_name, int value);

        std::vector<std::string> get_pipeline();
        void set_pipeline(const std::vector<std::string>& steps);

        // --- browser/instance
        
        void save_browsers(std::vector<std::shared_ptr<browser>> browsers);

        std::vector<std::shared_ptr<browser>> load_browsers();

        std::chrono::system_clock::time_point get_last_update_check_time();
        void set_last_update_check_time_to_now();


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
    };
}