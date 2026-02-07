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
#include <sqlite3.h>
#include "hashing.h"

using namespace std;
using hive = win32::reg::hive;
namespace fs = std::filesystem;
using json = nlohmann::json;

/**
 * @brief Executes an SQL query and returns results as a vector of row maps.
 * @param db Pointer to an open sqlite3 database connection.
 * @param query SQL query string to execute.
 * @return Vector of maps, where each map represents a row with column names as keys.
 */
vector<map<string, string>> sql_execute(sqlite3* db, const string& query) {
    vector<map<string, string>> results;

    sqlite3_stmt* stmt = nullptr;
    if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }

    int col_count = sqlite3_column_count(stmt);

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        map<string, string> row;
        for(int i = 0; i < col_count; ++i) {
            const char* col_name = sqlite3_column_name(stmt, i);
            const char* col_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[col_name ? col_name : ""] = col_value ? col_value : "";
        }
        results.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return results;
}

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
        // compute MD5 hash of the command, because just exe name is not unique enough, especially when almost every browser is named "chrome.exe"
        return hashing::md5(cmd);
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

    std::string discovery::unmangle_open_cmd(const std::string& open_cmd) {

        string r = open_cmd;

        // if open_cmd starts with quote ("), remove it, and substring up to first next quote
        if(r.starts_with("\"")) {
            r = r.substr(1);

            size_t pos = r.find("\"");
            if(pos != string::npos) {
                r = r.substr(0, pos);
            }
        }

        return r;
    }

    void discovery::discover_registry_browsers(hive h, vector<shared_ptr<browser>>& browsers, const string& ignore_proto) {
        auto subs = enum_subkeys(h, abs_root);

        for (const string& sub : subs) {
            string root = abs_root + "\\" + sub;
            string display_name = get_value(h, root);
            string open_command = get_value(h, root + "\\shell\\open\\command");
            open_command = unmangle_open_cmd(open_command);
            string http_url_assoc = get_value(h,
                root + "\\Capabilities\\URLAssociations", "http");

            if (!http_url_assoc.empty() && http_url_assoc != ignore_proto) {
                string id = get_id_from_open_cmd(open_command);
                auto b = make_shared<browser>(id, display_name, open_command);
                b->instance_id = get_instance_id(sub);
                b->is_autodiscovered = true;

                fingerprint(open_command, b->engine, b->data_path);

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
        if (!(b->is_autodiscovered && b->engine == browser_engine::chromium)) return;

        // https://github.com/ScoopInstaller/Extras/blob/5d9773cbeb8cbe7b1e97061cf4819b60956a3b61/bucket/helium.json#L22

        fs::path root{b->data_path};
        fs::path lsjf = root / "Local State";

        // faster method to discover
        if(fs::exists(lsjf)) {
            string jt = fss::read_all_text(str::to_str(lsjf));
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
            if(b->open_cmd.find("msedge.exe") != string::npos) {
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

    void discovery::discover_filefox_profile_groups(
        const string& parent_id,
        const string& installation_id,
        const string& store_id,
        const string& sqlite_db_path,
        const string& data_folder_path,
        std::vector<firefox_profile>& profiles) {

        // select * from Profiles
        sqlite3* db;
        int rc = sqlite3_open(sqlite_db_path.c_str(), &db);
        if(rc == SQLITE_OK) {
            auto sql_profiles = sql_execute(db, "select * from Profiles");
            for(const auto& profile : sql_profiles) {
                auto it_id = profile.find("id");
                auto it_name = profile.find("name");
                auto it_path = profile.find("path");

                string id = it_id != profile.end() ? it_id->second : "";
                string full_id = fmt::format("s{}i{}", store_id, id);
                string name = it_name != profile.end() ? it_name->second : full_id;
                string path = it_path != profile.end() ? it_path->second : "";
                path = (fs::path{data_folder_path} / path).string();

                profiles.emplace_back(full_id, parent_id, installation_id, name, path, false);
            }
        }
        sqlite3_close(db);
    }


    void discovery::discover_firefox_profiles(std::shared_ptr<browser> b, std::vector<firefox_profile>& profiles) {

        fs::path data_folder{b->data_path};

        // profiles.ini is the starting entry point to find both classic and new profiles (profile groups)
        fs::path ini_path = data_folder / "profiles.ini";
        if(!fs::exists(ini_path)) return;

        CSimpleIniA ini;
        ini.LoadFile(ini_path.c_str());
        list<CSimpleIniA::Entry> ir;
        ini.GetAllSections(ir);

        // create a map of profile id to installation id
        map<string, string> profile_to_installation_id;
        for(CSimpleIni::Entry& section : ir) {
            string section_name = section.pItem;
            if(!section_name.starts_with("Install")) continue;

            const char* c_profile_path = ini.GetValue(section.pItem, "Default");
            if(c_profile_path == nullptr) continue;

            string profile_path = c_profile_path;
            string installation_id = section_name.substr(strlen("Install"));

            if(!profile_path.empty() && !installation_id.empty()) {
                profile_to_installation_id[profile_path] = installation_id;
            }
        }

        // extract all the profiles
        for(CSimpleIniA::Entry& e : ir) {
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

            // get installation id if possible
            auto it_installation_id = profile_to_installation_id.find(path);
            string installation_id = it_installation_id != profile_to_installation_id.end()
                ? it_installation_id->second
                : "";

            if(is_relative) {
                path = (data_folder / path).string();
            }

            // Check is this is a container for profile groups
            const char* c_nested_store_id = ini.GetValue(e.pItem, "StoreID");
            if(c_nested_store_id) {
                fs::path sqlite_db_path = data_folder / "Profile Groups" / (string{c_nested_store_id} + ".sqlite");
                // profile definitions i.e. "new profiles" are now stored in the sqlite database
                if(fs::exists(sqlite_db_path)) {
                    discover_filefox_profile_groups(section_name,
                        installation_id,
                        c_nested_store_id,
                        sqlite_db_path.string(),
                        data_folder.string(),
                        profiles);
                }
            } else {
                // classic profile
                profiles.emplace_back(section_name, "", installation_id, display_name, path, true);
            }
        }
    }

    void discovery::discover_firefox_profiles(shared_ptr<browser> b) {
        if (!(b->is_autodiscovered && b->engine == browser_engine::gecko)) return;

        vector<firefox_profile> profiles;
        discover_firefox_profiles(b, profiles);

        // sort profiles using the following rules: is_classic, has installation_id, name
        std::sort(profiles.begin(), profiles.end(),
            [](const firefox_profile& a, const firefox_profile& b) {
                if(a.is_classic != b.is_classic) {
                    return !a.is_classic; // classic profiles last
                }
                if((!a.installation_id.empty()) != (!b.installation_id.empty())) {
                    return !a.installation_id.empty(); // profiles with installation_id first
                }
                return a.name < b.name; // finally by name
        });

        for(firefox_profile& fp : profiles) {

            // if profile is bound to an installation but it's not ours, skip it always
            if(!fp.installation_id.empty() && fp.installation_id != b->instance_id) continue;

            if(fp.installation_id.empty() && !g_config.discover_classic_firefox_profiles) continue;

            string arg_suffix = fp.is_classic
                ? fmt::format("-P \"{}\"", fp.name)
                : fmt::format("\"--profile\" \"{}\"", fp.path);
            string arg = fmt::format("\"{}\" {}", browser_instance::URL_ARG_NAME, arg_suffix);

            auto bi = make_shared<browser_instance>(b, fp.id, fp.name, arg, "");
            bi->sort_order = b->instances.size();
            b->instances.push_back(bi);

            // containers
            if(g_config.discover_firefox_containers) {
                // for each container, add a profile
                // Leave the "no container" profile as is.

                // add profile for each container
                vector<firefox_container> containers = discover_firefox_containers(fp.path);
                for(const auto& container : containers) {

                    string profile_name = fmt::format("{}::{}", fp.name, container.name);

                    string arg = fmt::format("\"ext+container:name={}&url={}\" {}",
                        container.name,
                        browser_instance::URL_ARG_NAME,
                        arg_suffix);

                    string id = fmt::format("{}+c_{}", fp.id, container.id);
                    auto bi = make_shared<browser_instance>(b, id, profile_name, arg, "");
                    bi->sort_order = b->instances.size();
                    b->instances.push_back(bi);
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
            string jt = fss::read_all_text(containers_path.string());
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
            string jt = fss::read_all_text(path.string());
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

        if(!(b->is_autodiscovered && b->instances.empty())) return;

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

    void discovery::get_default_browser_url_assoc(std::string& http, std::string& https) {
        http = get_shell_url_association_progid("http");
        https = get_shell_url_association_progid("https");
    }

    const std::vector<shared_ptr<browser>> discovery::discover_all_browsers() {
        return bt::discovery::discover_browsers(ProtoName);
    }

    string discovery::get_shell_url_association_progid(const string& protocol_name) {
        // There are 3 locations to check:
        // - HKEY_CURRENT_USER\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoiceLatest\ProgId, value of ProdId
        // - HKEY_CURRENT_USER\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoiceLatest, value of ProdId
        // - HKEY_CURRENT_USER\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoice, value of ProdId
        // If any of those are found, return the value

        // 1
        string prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            fmt::format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoiceLatest\\ProgId", protocol_name),
            "ProgId");
        if(!prog_id.empty()) return prog_id;

        // 2
        prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            fmt::format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoiceLatest", protocol_name),
            "ProgId");
        if(!prog_id.empty()) return prog_id;

        // 3
        prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            fmt::format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoice", protocol_name),
            "ProgId");
        return prog_id;
    }

    bool discovery::fingerprint(const std::string& exe_path, browser_engine& engine, std::string& data_path) {

        engine = browser_engine::unknown;
        data_path.clear();

        if(!fs::exists(exe_path)) return false;

        // get folder path
        fs::path p{exe_path};
        fs::path folder_path = p.parent_path();
        if(!fs::exists(folder_path)) return false;

        // get executable name
        auto exe_name = p.filename().string();
        str::lower(exe_name);

        // Chromium
        // Should have a file *_proxy.exe in the same folder
        for(const auto& entry : fs::directory_iterator(folder_path)) {
            if(entry.is_regular_file()) {
                auto filename = entry.path().filename().string();
                if(filename.ends_with("_proxy.exe")) {
                    engine = browser_engine::chromium;

                    // we don't know where data folders are exactly, so this is the best we can do

                    fs::path root = fs::path{lad};

                    if(exe_name == "msedge.exe") {
                        data_path = (root / "Microsoft" / "Edge" / "User Data").string();
                    } else if(exe_name == "chrome.exe") {
                        // Helium is also "chrome.exe", so we need to check path substring
                        if(str::contains_ic(exe_path, "\\Helium\\")) {
                            data_path = (root / "imput" / "Helium" / "User Data").string();
                        } else {
                            data_path = (root / "Google" / "Chrome" / "User Data").string();
                        }
                    } else if(exe_name == "vivaldi.exe") {
                        data_path = (root / "Vivaldi" / "User Data").string();
                    } else if(exe_name == "brave.exe") {
                        data_path = (root / "BraveSoftware" / "Brave-Browser" / "User Data").string();
                    } else if(exe_name == "thorium.exe") {
                        data_path = (root / "Thorium" / "User Data").string();
                    }

                    return true;
                }
            }
        }

        // Gecko
        // Should have "xul.dll" in the same folder
        for(const auto& entry : fs::directory_iterator(folder_path)) {
            if(entry.is_regular_file()) {
                auto filename = entry.path().filename().string();
                if(filename == "xul.dll") {
                    engine = browser_engine::gecko;

                    // as with chromium, we don't know exact data folder path

                    fs::path root = fs::path{ad};

                    if(exe_name == "firefox.exe") {
                        data_path = (root / "Mozilla" / "Firefox").string();
                    } else if(exe_name == "waterfox.exe") {
                        data_path = (root / "Waterfox").string();
                    } else if(exe_name == "librewolf.exe") {
                        data_path = (root / "Librewolf").string();
                    } else if(exe_name == "zen.exe") {
                        data_path = (root / "zen").string();
                    }

                    return true;
                }
            }
        }

        return false;
    }

    string get_settings_root() {
        return string("SOFTWARE\\") + APP_LONG_NAME;
    }
}