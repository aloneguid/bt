#include "setup.h"
#include <string>
#include "../../common/fss.h"
#include "../../common/win32/reg.h"
#include "../globals.h"
#include <fmt/core.h>
#include "discovery.h"
#include "win32/shell.h"

using namespace std;
using namespace win32::reg;

namespace bt {
    std::vector<system_check> setup::get_checks() {
        return vector<system_check> {
            system_check{
                "sys_browser", "System Browser",
                "Registered as a virtual browser in Windows.",
                "automatically register as a virtual browser",
                [](string& error_message) {
                    return setup::is_installed_as_browser(APP_LONG_NAME, error_message);
                },
                []() { setup::register_as_browser_and_custom_protocol(); return true; }},

                system_check{
                    "proto_http",
                    "HTTP Protocol Handler",
                    "Once set, Windows will forward HTTP links to it.",
                    "open system settings and set the default browser",
                    [](string& error_message) {
                        bool is_http, is_https, is_xbt;
                        discovery::is_default_browser(is_http, is_https, is_xbt);
                        if(!is_http) {
                            string http, https;
                            discovery::get_default_browser_url_assoc(http, https);
                            error_message = fmt::format("Current handler is {}.", http);
                        }
                        return is_http;
                    },
                    []() {
                        win32::shell::open_default_apps();
                        return true;
                    }
                },

                system_check{
                    "proto_https",
                    "HTTPS Protocol Handler",
                    "Once set, Windows will forward HTTPS links to it.",
                    "open system settings and set the default browser",
                    [](string& error_message) {
                        bool is_http, is_https, is_xbt;
                        discovery::is_default_browser(is_http, is_https, is_xbt);
                        if(!is_https) {
                            string http, https;
                            discovery::get_default_browser_url_assoc(http, https);
                            error_message = fmt::format("Current handler is {}.", https);
                        }
                        return is_https;
                    },
                    []() {
                        win32::shell::open_default_apps();
                        return true;
                    }
                }
        };
    }

    void setup::register_as_browser_and_custom_protocol() {
        register_protocol();

        register_browser();
    }

    void setup::unregister_all() {

        // unregister protocol
        string root = get_custom_proto_reg_path();
        win32::reg::delete_key(win32::reg::hive::current_user, root);

        // unregister browser
        root = get_browser_registration_reg_path();
        win32::reg::delete_key(win32::reg::hive::current_user, root);
    }

    std::string setup::get_browser_registration_reg_path() {
        return fmt::format("Software\\Clients\\StartMenuInternet\\{}", APP_LONG_NAME);
    }

    void setup::register_browser() {
        string app_path = fss::get_current_exec_path();

        string app_root = get_browser_registration_reg_path();
        string cap_root = fmt::format("{}\\Capabilities", app_root);

        set_value(hive::current_user, app_root, APP_LONG_NAME);

        //add capabilities
        set_value(hive::current_user, cap_root, APP_LONG_NAME, "ApplicationName");
        set_value(hive::current_user, cap_root, APP_REG_DESCRIPTION, "ApplicationDescription");
        set_value(hive::current_user, cap_root, app_path + ",0", "ApplicationIcon");

        //supported protocols
        vector<string> protocols{"https", "http", CustomProtoName};
        for(const string& protocol : protocols) {
            set_value(hive::current_user, cap_root + "\\URLAssociations", ProtoName, protocol);
        }

        //supported file extensions
        vector<string> html_exts{".svg", ".htm", ".html", ".shtml", ".webp", ".xht", ".xhtml", ".mht", ".mhtml"};
        vector<string> pdf_exts{".pdf"};
        delete_key(hive::current_user, cap_root + "\\FileAssociations");
        for(const string& ext : html_exts) {
            set_value(hive::current_user, cap_root + "\\FileAssociations", ProtoName, ext);
        }
        for(const string& ext : pdf_exts) {
            set_value(hive::current_user, cap_root + "\\FileAssociations", PdfProtoName, ext);
        }

        //icon and command
        set_value(hive::current_user,
           app_root + "\\DefaultIcon",
           app_path + ",0");
        set_value(hive::current_user,
           app_root + "\\shell\\open\\command",
           fmt::format("\"{}\"", app_path));

        //register caps
        set_value(hive::current_user, "Software\\RegisteredApplications", cap_root, APP_LONG_NAME);

        //register HTML handler
        string handler_path = string("Software\\Classes\\") + ProtoName;
        set_value(hive::current_user, handler_path, string(APP_LONG_NAME) + " HTML Document");
        set_value(hive::current_user, handler_path + "\\DefaultIcon", app_path + ",0");
        set_value(hive::current_user, handler_path + "\\Application", APP_LONG_NAME, "ApplicationName");
        set_value(hive::current_user, handler_path + "\\Application", APP_REG_DESCRIPTION, "ApplicationDescription");
        set_value(hive::current_user, handler_path + "\\shell\\open\\command",
           string("\"") + app_path + "\" %1");

        //register PDF handler
        handler_path = string("Software\\Classes\\") + PdfProtoName;
        set_value(hive::current_user, handler_path, string(APP_LONG_NAME) + " PDF Document");
        set_value(hive::current_user, handler_path + "\\DefaultIcon", app_path + ",1");
        set_value(hive::current_user, handler_path + "\\Application", APP_LONG_NAME, "ApplicationName");
        set_value(hive::current_user, handler_path + "\\Application", APP_REG_DESCRIPTION, "ApplicationDescription");
        set_value(hive::current_user, handler_path + "\\shell\\open\\command",
           string("\"") + app_path + "\" %1");
    }

    std::string setup::get_custom_proto_reg_path() {
        return fmt::format("Software\\Classes\\{}", CustomProtoName);
    }

    void setup::register_protocol() {
        string app_path = fss::get_current_exec_path();

        string root = get_custom_proto_reg_path();

        set_value(hive::current_user, root, fmt::format("URL:{}", CustomProtoName));
        set_value(hive::current_user, root, "", "URL Protocol");

        string command_root = fmt::format("{}\\shell\\open\\command", root);
        string open_command = fmt::format("\"{}\" \"%1\"", app_path);
        set_value(hive::current_user, command_root, open_command);
    }

    void setup::uninstall_as_browser(const std::string& proto_name, const std::string& name) {
        string root = "Software\\Clients\\StartMenuInternet";
        string app_root = root + "\\" + name;
        string cap_root = app_root + "\\Capabilities";
        string handler_path = string("Software\\Classes\\") + proto_name;

        delete_key(hive::current_user, app_root);
        delete_value(hive::current_user, "Software\\RegisteredApplications", name);
        delete_key(hive::current_user, handler_path);
    }

    bool setup::is_installed_as_browser(const std::string& name, string& error_message) {
        string root = "Software\\Clients\\StartMenuInternet";
        string app_root = root + "\\" + name;
        string soc = app_root + "\\shell\\open\\command";

        string app_path = fmt::format("\"{}\"", fss::get_current_exec_path());
        string reg_app_path = get_value(hive::current_user, soc);
        bool installed = app_path == reg_app_path;
        if(installed) {
            error_message.clear();
        } else {
            error_message = fmt::format("Expected: {}\nRegistered: {}.", app_path, reg_app_path);
        }
        return installed;
    }
}