#include "discovery.h"
#include "fss.h"
#include "config.h"
#include "str.h"
#include "win32/shell.h"
#include "win32/reg.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <fmt/core.h>
#include "../globals.h"
#include <SimpleIni.h> // https://github.com/brofield/simpleini
#include <tinyxml2.h>

using namespace std;
using hive = win32::reg::hive;
namespace fs = std::filesystem;
using json = nlohmann::json;

// mapping of vendors to vendor data folder (relative to AppData\Local)
// Arc stores data under AppData\Local\Packages\TheBrowserCompany.Arc_ttt1ap7aakyb4\LocalCache\Local\Arc\User Data

struct browser_info_entry {
    // Vendor Data Folder, relative to AppData\Local
    string vdf;

    // Optional path substring to help identify identical IDs
    string path_substr;
};

struct browser_info {
    vector<browser_info_entry> entries;

    browser_info_entry& get_best_entry(const string& open_cmd) {
        for(auto& entry : entries) {
            if(!entry.path_substr.empty() && str::contains_ic(open_cmd, entry.path_substr)) {
                return entry;
            }
        }
        return entries[0];
    }
};

map<string, browser_info> chromium_id_to_bi{
    { "msedge", browser_info {
            { browser_info_entry{"Microsoft\\Edge\\User Data"} }
        }
    },
    { "chrome", browser_info {
            {
                browser_info_entry{"Google\\Chrome\\User Data"},
                browser_info_entry{"imput\\Helium\\User Data", "\\Helium\\" }
            }
        }
    },
    { "vivaldi", browser_info {
            { browser_info_entry{"Vivaldi\\User Data"} }
        }
    },
    { "brave", browser_info {
            { browser_info_entry{"BraveSoftware\\Brave-Browser\\User Data"} }
        }
    },
    { "thorium", browser_info {
            { browser_info_entry{"Thorium\\User Data"} }
        }
    }
};

map<string, string> firefox_id_to_vdf{
    { "firefox", "Mozilla\\Firefox" },
    { "Mozilla.Firefox_n80bbvh6b1yt2!App", "Mozilla\\Firefox" },    // Microsoft Store version
    { "waterfox", "Waterfox" },
    { "librewolf", "Librewolf" },
    { "zen", "zen" }
};

namespace bt {
    const string abs_root = "SOFTWARE\\Clients\\StartMenuInternet";
    const string apps_root = "Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\PackageRepository\\Packages";
    const string app_user_root = "Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\Repository\\Packages";
    const string ad = win32::shell::get_app_data_folder();
    const string lad = win32::shell::get_local_app_data_path();
    const string FirefoxInstancePrefix = "Firefox-";
    // any parameters to add to Chromium-based browsers
    const string ChromiumExtraArgs = " --no-default-browser-check";

    string get_id_from_open_cmd(const std::string& cmd) {
        fs::path p(cmd);
        p.replace_extension();
        return p.filename().string();
    }

    bool is_msstore_browser(const string& package_name, string& app_path, string& app_user_model_id) {

        app_path = win32::reg::get_value(hive::classes_root, apps_root + "\\" + package_name, "Path");

        // the first child, if present, is the "app user model id"
        auto subkeys = win32::reg::enum_subkeys(hive::classes_root, apps_root + "\\" + package_name);
        if(subkeys.empty()) return false;

        // the first child, if present, is the "app user model id"
        app_user_model_id = subkeys[0];

        // it's sufficient to check for the availability of the "https" key to make the decision
        string https_key_path = fmt::format("{}\\{}\\{}\\windows.protocol\\https", apps_root, package_name, app_user_model_id);
        return win32::reg::path_exists(hive::classes_root, https_key_path);
    }

    string get_appx_app_id(const string& package_name) {
        string path = app_user_root + "\\" + package_name;
        auto usubs = win32::reg::enum_subkeys(hive::current_user, path);
        if(usubs.empty()) return "";

        string appx_id = win32::reg::get_value(hive::current_user,
            fmt::format("{}\\{}\\Capabilities\\URLAssociations", path, usubs[0]),
            "https");

        return appx_id;
    }

    void read_appx_manifest(const string& app_folder, string& display_name, string& icon_path) {
        string manifest_path = app_folder + "\\AppxManifest.xml";

        tinyxml2::XMLDocument doc;
        doc.LoadFile(manifest_path.c_str());

        tinyxml2::XMLElement* x_root = doc.FirstChildElement("Package");
        if(!x_root) return;

        tinyxml2::XMLElement* x_properties = x_root->FirstChildElement("Properties");
        if(!x_properties) return;

        // try to get display name
        tinyxml2::XMLElement* x_display_name = x_properties->FirstChildElement("DisplayName");
        if(x_display_name) {
            display_name = x_display_name->GetText();
        }

        // try to get icon path
        tinyxml2::XMLElement* x_logo = x_properties->FirstChildElement("Logo");
        if(x_logo) {
            string logo = x_logo->GetText();
            icon_path = app_folder + "\\" + logo;

            // check if the file exists, and if not, try to find the best match
            if(!fs::exists(icon_path)) {
                // try to find the best match
                fs::path p{icon_path};
                icon_path.clear();  // clear result as it's not valid
                string ext = p.extension().string();
                string name = p.stem().string();
                string dir = p.parent_path().string();

                // try to find the best match
                for(const string& candidate : {".scale-100", ".scale-125", ".scale-150", ".scale-200"}) {
                    auto candidate_path = fs::path{dir} / (name + candidate + ext);
                    if(fs::exists(candidate_path)) {
                        icon_path = candidate_path.string();
                        break;
                    }
                }
            }
        }
    }

    void discover_msstore_browsers(vector<shared_ptr<browser>>& browsers) {

        // hint: this might be easier for future HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppModel\PackageRepository\Extensions\windows.protocol\https

        // 1. List packages under HKCU\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppModel\PackageRepository\Packages

        auto package_names = win32::reg::enum_subkeys(hive::classes_root, apps_root);

        for(const string& package_name : package_names) {

            string app_folder;  // this is where the app is installed
            string app_user_model_id;  // this is the app user model id

            if(!is_msstore_browser(package_name, app_folder, app_user_model_id)) continue;
            string appx_id = get_appx_app_id(package_name);

            // get properties from manifest
            string display_name, icon_path;
            read_appx_manifest(app_folder, display_name, icon_path);

            if(!appx_id.empty()) {
                // go to HKCR/id to get shell/open/command
                string open_command = win32::reg::get_value(hive::classes_root,
                    fmt::format("{}\\shell\\open\\command", appx_id));

                // this command can be used to set full path to executable (de-obfuscated in "browser" constructor)
                auto b = make_shared<browser>(app_user_model_id, display_name, open_command);
                b->icon_path = icon_path;

                if(!b->is_wellknown()) {
                    // not sure how to handle it, so we will use msstore open method

                    // get app family id, which is model id without the "!" and everything after it
                    string app_family_id = app_user_model_id.substr(0, app_user_model_id.find('!'));

                    b->open_cmd = fmt::format("{}{}", browser::UwpCmdPrefix, app_family_id);
                }

                browsers.push_back(b);
            }
        }
    }

    string get_instance_id(const string& reg_value) {
        // if this is Firefox, strip out the prefix to get instance ID
        if(reg_value.starts_with(FirefoxInstancePrefix)) {
            return reg_value.substr(FirefoxInstancePrefix.size());
        }

        return reg_value;
    }

    void discover_registry_browsers(hive h, vector<shared_ptr<browser>>& browsers, const string& ignore_proto) {
        auto subs = enum_subkeys(h, abs_root);

        for (const string& sub : subs) {
            string root = abs_root + "\\" + sub;
            string display_name = get_value(h, root);
            string open_command = get_value(h, root + "\\shell\\open\\command");
            string http_url_assoc = get_value(h,
                root + "\\Capabilities\\URLAssociations", "http");

            if (!http_url_assoc.empty() && http_url_assoc != ignore_proto) {
                string id = get_id_from_open_cmd(open_command);
                auto b = make_shared<browser>(id, display_name, open_command);
                b->disco_instance_id = get_instance_id(sub);

                // check for duplicates (HKLM & HKCU can have the same browser registered)
                // this is possible to to operator== on browser class

                bool is_dupe{false};
                for(auto bb : browsers) {
                    if(bb->id == b->id) {
                        is_dupe = true;
                        break;
                    }
                }
                if(!is_dupe) {
                    browsers.push_back(b);
                }

            }
        }
    }

    std::vector<shared_ptr<browser>> discovery::discover_browsers(const std::string& ignore_proto) {
        vector<shared_ptr<browser>> browsers;

        discover_registry_browsers(hive::local_machine, browsers, ignore_proto);
        discover_registry_browsers(hive::current_user, browsers, ignore_proto);
        discover_msstore_browsers(browsers);

        // discover various profiles
        for (shared_ptr<bt::browser> b : browsers) {
            discover_chrome_profiles(b);
            discover_firefox_profiles(b);
            discover_other_profiles(b);
        }

        return browsers;
    }

    void discovery::discover_chrome_profiles(shared_ptr<browser> b) {
        if (!(b->is_system && b->is_chromium)) return;

        if(!chromium_id_to_bi.contains(b->id)) return;

        auto bi = chromium_id_to_bi[b->id];
        auto bie = bi.get_best_entry(b->open_cmd);

        fs::path root = fs::path{lad} / bie.vdf;
        fs::path lsjf = root / "Local State";

        // faster method to discover
        if(fs::exists(lsjf)) {
            string jt = fss::read_file_as_string(str::to_str(lsjf));
            auto j = json::parse(jt);
            auto j_p_ic = j["profile"]["info_cache"];
            if(j_p_ic.is_object()) {
                for(auto& jp : j_p_ic.items()) {
                    string sys_name = jp.key();
                    auto profile_pic_j = jp.value()["gaia_picture_file_name"];

                    string name;
                    {
                        auto j = jp.value()["shortcut_name"];
                        if(j.is_string()) {
                            name = j.get<string>();
                        }

                        if(name.empty()) {
                            j = jp.value()["name"];
                            if(j.is_string()) {
                                name = j.get<string>();
                            }
                        }
                    }

                    // all the data is ready
                    string arg = fmt::format("\"{}\" \"--profile-directory={}\"{}",
                        browser_instance::URL_ARG_NAME, sys_name,
                        ChromiumExtraArgs);

                    auto bi = make_shared<browser_instance>(
                        b,
                        sys_name,
                        name,
                        arg,
                        ""
                    );
                    bi->sort_order = b->instances.size();
                    if(profile_pic_j.is_string()) {
                        bi->icon_path = (root / sys_name / profile_pic_j.get<string>()).string();
                    }
                    b->instances.push_back(bi);
                }
            }
        }

        {
            if(b->id == "msedge") {
                // Edge is not stupid, it's just different
                auto inprivate = make_shared<browser_instance>(
                    b, "InPrivate", "InPrivate",
                    fmt::format("\"{}\" --inprivate", browser_instance::URL_ARG_NAME),
                    "");
                inprivate->is_incognito = true;
                inprivate->sort_order = b->instances.size();
                b->instances.push_back(inprivate);
            } else {
                auto inprivate = make_shared<browser_instance>(
                b, "Incognito", "Incognito",
                fmt::format("\"{}\" --incognito", browser_instance::URL_ARG_NAME),
                "");
                inprivate->is_incognito = true;
                inprivate->sort_order = b->instances.size();
                b->instances.push_back(inprivate);
            }
        }

        // Brave additionally supports Tor mode
        if(b->id == "brave") {
            auto tor = make_shared<browser_instance>(
                b, "tor", "Tor",
                fmt::format("\"{}\" --tor", browser_instance::URL_ARG_NAME),
                ""
            );
            tor->is_incognito = true;
            tor->sort_order = b->instances.size();
            b->instances.push_back(tor);
        }
    }

    void discovery::discover_firefox_profiles(shared_ptr<browser> b) {
        if (!(b->is_system && b->is_firefox)) return;

        {
            // see http://kb.mozillazine.org/Profiles.ini_file

            if(!firefox_id_to_vdf.contains(b->id)) return;
            string vdf = firefox_id_to_vdf[b->id];

            fs::path ini_path = fs::path{ad} / vdf / "profiles.ini";
            if(fs::exists(ini_path)) {
                CSimpleIniA ini;
                ini.LoadFile(ini_path.c_str());
                list<CSimpleIniA::Entry> ir;
                ini.GetAllSections(ir);

                // find section with installation ID to extract default profile path for this Firefox installation
                string section_name = "Install" + b->disco_instance_id;
                string default_profile_path;
                for(CSimpleIni::Entry& section : ir) {
                    if(section_name == section.pItem) {
                        const char* c_path = ini.GetValue(section.pItem, "Default");
                        if(c_path) {
                            default_profile_path = c_path;
                        }
                        break;
                    }
                }

                for(auto& e : ir) {
                    // a section is a profile if starts with "Profile".
                    string section_name{e.pItem};
                    if(!section_name.starts_with("Profile")) continue;

                    // extract display name if possible
                    const char* c_name = ini.GetValue(e.pItem, "Name");
                    string display_name = c_name ? c_name : section_name;

                    // if is_relative is false, this is an absolute path (not like it matters anyway)
                    const char* c_is_relative = ini.GetValue(e.pItem, "IsRelative");
                    const char* c_path = ini.GetValue(e.pItem, "Path");
                    bool is_relative = c_is_relative == nullptr || string{c_is_relative} == "1";
                    if(!c_path) continue;
                    string path{c_path};

                    bool is_default_profile = path == default_profile_path;

                    // We don't want "default" profile - in Firefox it's some kind of pre-release oddness.
                    // In Waterfox it's called something like "68-edition-default"
                    //if(display_name == "default" || display_name.ends_with("-edition-default")) continue;

                    string local_home = is_relative
                        ? (fs::path{lad} / vdf / c_path).string()
                        : c_path;

                    string roaming_home = is_relative
                        ? (fs::path{ad} / vdf / c_path).string()
                        : c_path;

                    string arg = fmt::format("\"{}\" -P \"{}\"", browser_instance::URL_ARG_NAME, display_name);
                    auto bi = make_shared<browser_instance>(b,
                        e.pItem,
                        display_name,
                        arg,
                        "");
                    bi->sort_order = is_default_profile ? -1 : b->instances.size();

                    {
                        const char* c_default = ini.GetValue(e.pItem, "Default");
                        bi->is_default = c_default != nullptr && string{c_default} == "1";
                    }

                    b->instances.push_back(bi);

                    if(g_config.discover_firefox_containers) {
                        // for each container, add a profile
                        // Leave the "no container" profile as is.

                        // add profile for each container
                        vector<firefox_container> containers = discover_firefox_containers(roaming_home);
                        for(const auto& container : containers) {

                            string profile_name = fmt::format("{}::{}", display_name, container.name);

                            string arg = fmt::format("\"ext+container:name={}&url={}\" -P \"{}\"",
                                container.name,
                                browser_instance::URL_ARG_NAME,
                                display_name);


                            string id = fmt::format("{}+c_{}", e.pItem, container.id);
                            auto bi = make_shared<browser_instance>(b, id, profile_name, arg, "");
                            bi->sort_order = b->instances.size();
                            b->instances.push_back(bi);
                        }

                    }
                }
            }
        }

        // Safe Mode starts FF without extensions
        //auto safe_bi = browser_instance(b, "safemode", "Safe Mode",
        //   arg + " -safe-mode",
        //   b.open_cmd);

        // in-private
        auto private_bi = make_shared<browser_instance>(b, "private", "Private",
            fmt::format("-private-window \"{}\"", browser_instance::URL_ARG_NAME),
            b->open_cmd);
        private_bi->is_incognito = true;
        private_bi->sort_order = b->instances.size();

        b->instances.push_back(private_bi);
    }

    vector<firefox_container> discovery::discover_firefox_containers(const string& roaming_home) {

        vector<firefox_container> r;

        // detect if "containers" are installed
        fs::path containers_path = fs::path{roaming_home} / "containers.json";
        if(fs::exists(containers_path)) {
            string jt = fss::read_file_as_string(containers_path.string());
            auto j = json::parse(jt);
            auto identities = j["identities"];
            if(identities.is_array()) {
                for(json::iterator it = identities.begin(); it != identities.end(); ++it) {
                    auto identity = *it;
                    auto j_is_public = identity["public"];
                    if(!j_is_public.is_boolean() || !j_is_public.get<bool>()) continue;

                    auto j_id = identity["userContextId"];
                    if(!j_id.is_number()) continue;

                    auto j_name = identity["name"];
                    auto j_l10nID = identity["l10nID"];
                    auto j_l10nId = identity["l10nId"];
                    //string t0 = j_name.type_name();
                    //string t1 = j_l10nID.type_name();

                    if(!(j_name.is_string() || j_l10nID.is_string() || j_l10nId.is_string())) continue;

                    int id = j_id.get<int>();
                    string name;
                    if(j_name.is_string()) {
                        name = j_name.get<string>();
                    } else {
                        // there are 4 default containers - Personal, Work, Banking, Shopping.
                        // They can be deleted, or renamed (in which case they will get "name" property set)

                        string lid = j_l10nID.is_string() ? j_l10nID.get<string>() : j_l10nId.get<string>();
                        if(lid == "userContextPersonal.label" || lid == "user-context-personal") {
                            name = "Personal";
                        } else if(lid == "userContextWork.label" || lid == "user-context-work") {
                            name = "Work";
                        } else if(lid == "userContextBanking.label" || lid == "user-context-banking") {
                            name = "Banking";
                        } else if(lid == "userContextShopping.label" || lid == "user-context-shopping") {
                            name = "Shopping";
                        } else {
                            name = lid;
                        }
                    }

                    r.emplace_back(to_string(id), name);
                }
            }
        }

        return r;
    }

    std::vector<std::string> discovery::get_firefox_addons_installed(const std::string& roaming_home) {
        vector<string> r;

        fs::path path = fs::path{roaming_home} / "addons.json";
        if(fs::exists(path)) {
            string jt = fss::read_file_as_string(path.string());
            auto j = json::parse(jt);
            auto j_addons = j["addons"];
            if(j_addons.is_array()) {
                for(json::iterator it = j_addons.begin(); it != j_addons.end(); ++it) {
                    auto j_addon = *it;
                    auto j_id = j_addon["id"];
                    if(!j_id.is_string()) continue;
                    r.push_back(j_id.get<string>());
                }
            }
        }


        return r;
    }

    void discovery::discover_other_profiles(shared_ptr<browser> b) {

        if(!(b->is_system && b->instances.empty())) return;

        string icon_path = b->icon_path.empty() ? b->open_cmd : b->icon_path;


        if(b->is_msstore()) {
            auto bi = make_shared<browser_instance>(b, "default", "Default",
                browser_instance::URL_ARG_NAME,
                icon_path);

            b->instances.push_back(bi);
        } else {
            string arg("\"");
            arg += browser_instance::URL_ARG_NAME;
            arg += "\"";

            auto bi = make_shared<browser_instance>(b, "default", "Default",
                arg,
                icon_path);

            b->instances.push_back(bi);
        }
    }

    bool discovery::is_default_browser(bool& http, bool& https, bool& xbt) {
        http = ProtoName == get_shell_url_association_progid("http");
        https = ProtoName == get_shell_url_association_progid("https");

        auto xbt_assoc = get_shell_url_association_progid(CustomProtoName);
        xbt = xbt_assoc.empty() || ProtoName == xbt_assoc;

        return http && https && xbt;
    }

    const std::vector<shared_ptr<browser>> discovery::discover_all_browsers() {
        return bt::discovery::discover_browsers(ProtoName);
    }

    bool discovery::is_chromium_id(const std::string& system_id) {
        return chromium_id_to_bi.find(system_id) != chromium_id_to_bi.end();
    }

    bool discovery::is_firefox_id(const std::string& system_id) {
        return firefox_id_to_vdf.find(system_id) != firefox_id_to_vdf.end();
    }

    string discovery::get_shell_url_association_progid(const string& protocol_name) {
        string prog_id;

        prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            fmt::format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoice", protocol_name),
            "ProgId");

        if(!prog_id.empty()) return prog_id;

        // Since some version of Windows 11, UserChoiceLatest is used instead of UserChoice
        prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            fmt::format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoiceLatest", protocol_name),
            "ProgId");

        return prog_id;
    }

    string get_settings_root() {
        return string("SOFTWARE\\") + APP_LONG_NAME;
    }
}