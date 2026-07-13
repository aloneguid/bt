#include "setup.h"
#include <string>
#include "fss.h"
#include "../globals.h"
#include <format>
#include "discovery.h"

#if PLATFORM_WINDOWS
#include "win32/reg.h"
#include "win32/shell.h"

using namespace grey::common;
using namespace grey::common::win32::reg;

#endif

using namespace std;

namespace bt {
    std::vector<system_check> setup::get_checks() {

#if PLATFORM_WINDOWS
        return vector {
                system_check{
                    "default_browser",
                    "System Default Browser",
                    "Once set, Windows will forward HTTP(S) links to it.",
                    "open system settings where you can set the default browser",
                    [](string& error_message) {
                        string progid_http = get_shell_url_association_progid("http");
                        string progid_https = get_shell_url_association_progid("https");
                        bool myself_http = progid_http == ProtoName;
                        bool myself_https = progid_https == ProtoName;
                        bool myself_is_default = myself_http && myself_https;

                        if(!myself_is_default) {
                            error_message = format(
                                "Current system settings:\nHTTP:  {} ({})\nHTTPS: {} ({})\nExpected progid: {}",
                                progid_http,
                                get_progid_application_name(progid_http),
                                progid_https,
                                get_progid_application_name(progid_https),
                                ProtoName);
                        }
                        return myself_is_default;
                    },
                    []() {
                        win32::shell::open_default_apps(APP_LONG_NAME, true);
                        return true;
                    }
                }
        };
#else
        return vector<system_check>();
#endif
    }

#if PLATFORM_WINDOWS
    string setup::get_shell_url_association_progid(const string& protocol_name) {
        // There are 3 locations to check:
        // - HKEY_CURRENT_USER\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoiceLatest\ProgId, value of ProdId
        // - HKEY_CURRENT_USER\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoiceLatest, value of ProdId
        // - HKEY_CURRENT_USER\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoice, value of ProdId
        // If any of those are found, return the value

        // 1
        string prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoiceLatest\\ProgId",
                protocol_name),
            "ProgId");
        if(!prog_id.empty()) return prog_id;

        // 2
        prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoiceLatest",
                protocol_name),
            "ProgId");
        if(!prog_id.empty()) return prog_id;

        // 3
        prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoice", protocol_name),
            "ProgId");
        return prog_id;
    }

    std::string setup::get_progid_application_name(const std::string& prog_id) {
        string app_name = win32::reg::get_value(
            hive::classes_root,
            format("{}\\Application", prog_id),
            "ApplicationName");

        if(app_name.empty()) {
            app_name = win32::reg::get_value(
                hive::classes_root,
                format("{}\\shell\\open\\command", prog_id));
        }

        if(app_name.empty()) {
            app_name = prog_id;
        }

        return app_name;
    }

#endif

}