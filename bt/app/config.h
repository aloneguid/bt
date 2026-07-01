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

    enum class icon_overlay_mode : unsigned int {
        profile_on_browser  = 0,
        browser_on_profile  = 1,
        browser_only        = 2,
        profile_only        = 3
    };

    class config {
    public:

        // -------------- begin

        icon_overlay_mode icon_overlay{icon_overlay_mode::profile_on_browser};

        // browser collection
        std::vector<std::shared_ptr<browser>> browsers;

        // ------------- end

        config();
        void commit();

        std::string get_absolute_path();

        // experimental flags
        std::string get_flag(const std::string& name);

        // conversion functions
        static std::string icon_overlay_mode_to_string(icon_overlay_mode mode);
        static icon_overlay_mode to_icon_overlay_mode(const std::string& name);
        static std::string browser_engine_to_string(browser_engine engine);
        static browser_engine to_browser_engine(const std::string& name);

        static std::string get_data_file_path(const std::string& name);

    private:
        ::common::config cfg;

        void migrate();
        void load();

        // --- browser/instance

        void save_browsers(std::vector<std::shared_ptr<browser>> browsers);
        std::vector<std::shared_ptr<browser>> load_browsers();
    };
}