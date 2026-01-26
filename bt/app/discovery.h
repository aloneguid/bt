#pragma once
#include "browser.h"
#include "win32/reg.h"
#include <vector>

namespace bt {

    struct firefox_profile {
        std::string id;
        std::string parent_id;  // parent profile id, if this is a "new profile" aka inside a "profile group"
        std::string installation_id;
        std::string name;
        std::string path;
        bool is_classic{true};
    };

    struct firefox_container {
        std::string id;
        std::string name;
    };

    /**
     * @brief Contains various methods to auto-discover browsers installed on the system.
     */
    class discovery {
    public:
        static bool is_default_browser(bool& http, bool& https, bool& xbt);

        /**
         * @brief Scans the system for all the browsers.Also returns custom browser placeholder.
         */
        static const std::vector<std::shared_ptr<browser>> discover_all_browsers();

    private:
        inline static const int ICON_SIZE = 256;

        static std::vector<std::shared_ptr<browser>> discover_browsers(const std::string& ignore_proto);

        static void discover_chrome_profiles(std::shared_ptr<browser> b);

        static void discover_firefox_profiles(std::shared_ptr<browser> b, std::vector<firefox_profile>& profiles);

        static void discover_firefox_profiles(std::shared_ptr<browser> b);

        static void discover_filefox_profile_groups(
            const std::string& parent_id,
            const std::string& installation_id,
            const std::string& store_id,
            const std::string& sqlite_db_path,
            const std::string& data_folder_path,
            std::vector<firefox_profile>& profiles);

        static std::vector<firefox_container> discover_firefox_containers(const std::string& roaming_home);

        static std::vector<std::string> get_firefox_addons_installed(const std::string& roaming_home);

        static void discover_other_profiles(std::shared_ptr<browser> b);

        static std::string get_shell_url_association_progid(const std::string& protocol_name = "http");

        static std::string unmangle_open_cmd(const std::string& open_cmd);

        static void discover_registry_browsers(win32::reg::hive h,
            std::vector<std::shared_ptr<browser>>& browsers, const std::string& ignore_proto);

        /**
         * @brief Performs browser fingerprinting based on the executable path.
         * @param exe_path 
         * @param engine Engine type output
         * @param data_path Data path output
         * @return 
         */
        static bool fingerprint(const std::string& exe_path, browser_engine& engine, std::string& data_path);
    };
}