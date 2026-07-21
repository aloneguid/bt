#pragma once
#include "browser.h"
#include <vector>
#include "common/platform.h"

#if PLATFORM_WINDOWS
// Windows discovery is nice - all the browsers register in the windows registry
#include "common/win32/reg.h"
#endif

namespace bt {
    struct firefox_profile {
        std::string id;
        std::string parent_id; // parent profile id, if this is a "new profile" aka inside a "profile group"
        std::string installation_id;
        std::string name;
        std::string path;
        bool is_classic{true};
    };

    struct firefox_container {
        std::string id;
        std::string name;
        std::string icon_name;
        std::string color_name;
    };

    /**
     * @brief Contains various methods to auto-discover browsers installed on the system.
     */
    class discovery {
    public:
        /**
         * @brief Scans the system for all the browsers.Also returns custom browser placeholder.
         */
        static std::vector<browser> discover_all_browsers();

        static void discover_managed_profiles(std::vector<browser> &browsers);

        static void discover_managed_profiles(browser &browser);

        static void discover_other_profiles(browser &b);

    private:
        static constexpr int ICON_SIZE = 256;

        static std::vector<browser> discover_browsers(const std::string &ignore_proto);

        static void discover_chrome_profiles(browser &b);

        static void discover_gecko_profiles(browser &b, std::vector<firefox_profile> &profiles);

        static void discover_gecko_profiles(browser &b);

        static void discover_gecko_profile_groups(
            const std::string &parent_id,
            const std::string &installation_id,
            const std::string &store_id,
            const std::string &sqlite_db_path,
            const std::string &data_folder_path,
            std::vector<firefox_profile> &profiles);

        static std::vector<firefox_container> discover_gecko_containers(const std::string &roaming_home);

        static std::vector<std::string> get_firefox_addons_installed(const std::string &roaming_home);

        static std::string unmangle_open_cmd(const std::string &open_cmd);

#if PLATFORM_WINDOWS

        static void discover_win32_registry_browsers(grey::common::win32::reg::hive h,
                                                     std::vector<browser> &browsers, const std::string &ignore_proto);
#endif

#if PLATFORM_LINUX
        static std::string resolve_xdg_icon_path(const std::string &icon);

        static void discover_xdg_desktop_browsers(std::vector<browser> &browsers);
#endif

        /**
         * @brief Performs browser fingerprinting based on the executable path.
         * @param exe_path
         * @param engine Engine type output
         * @param data_path Data path output
         * @return
         */
        static bool fingerprint(const std::string &exe_path, browser_engine &engine, std::string &data_path);
    };
}
