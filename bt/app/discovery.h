#pragma once
#include "browser.h"
#include <vector>

namespace bt
{
    class discovery {
    public:
        static bool is_default_browser(bool& http, bool& https, bool& xbt);

        // Scans the system for all the browsers. Also returns custom browser placeholder.
        static const std::vector<std::shared_ptr<browser>> discover_all_browsers();

    private:
        inline static const int ICON_SIZE = 256;

        static std::vector<std::shared_ptr<browser>> discover_browsers(const std::string& ignore_proto);

        static void discover_chrome_profiles(std::shared_ptr<browser> b);

        static void discover_firefox_profiles(std::shared_ptr<browser> b);

        static void discover_other_profiles(std::shared_ptr<browser> b);

        static std::string get_shell_url_association_progid(const std::string& protocol_name = "http");
    };
}