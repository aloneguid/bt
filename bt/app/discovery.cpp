#include "discovery.h"
#include "common/fss.h"
#include "common/str.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <format>
#include "../globals.h"
#include <SimpleIni.h> // https://github.com/brofield/simpleini
#include <tinyxml2.h>
#include <sqlite3.h>
#include <unordered_set>
#include <fstream>

using namespace grey::common;


#if PLATFORM_WINDOWS
#include "common/win32/shell.h"
#include "common/win32/reg.h"
using hive = win32::reg::hive;
#endif

using namespace std;

namespace fs = std::filesystem;
using json = nlohmann::json;

/**
 * @brief Executes an SQL query and returns results as a vector of row maps.
 * @param db Pointer to an open sqlite3 database connection.
 * @param query SQL query string to execute.
 * @return Vector of maps, where each map represents a row with column names as keys.
 */
vector<map<string, string> > sql_execute(sqlite3 *db, const string &query) {
    vector<map<string, string> > results;

    sqlite3_stmt *stmt = nullptr;
    if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }

    int col_count = sqlite3_column_count(stmt);

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        map<string, string> row;
        for(int i = 0; i < col_count; ++i) {
            const char *col_name = sqlite3_column_name(stmt, i);
            const char *col_value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            row[col_name ? col_name : ""] = col_value ? col_value : "";
        }
        results.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return results;
}

namespace bt {
    const string abs_root = "SOFTWARE\\Clients\\StartMenuInternet";
    const string app_user_root =
            "Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\Repository\\Packages";
#if PLATFORM_WINDOWS
    const string ad = win32::shell::get_app_data_folder();
    const string lad = win32::shell::get_local_app_data_path();
#else
    const string ad = "todo";
    const string lad = "todo";
#endif

    const string FirefoxInstancePrefix = "Firefox-";

    static const std::unordered_map<std::string, std::string> gecko_container_names = {
        {"userContextPersonal.label", "Personal"},
        {"user-context-personal", "Personal"},
        {"userContextWork.label", "Work"},
        {"user-context-work", "Work"},
        {"userContextBanking.label", "Banking"},
        {"user-context-banking", "Banking"},
        {"userContextShopping.label", "Shopping"},
        {"user-context-shopping", "Shopping"}
    };

    static const std::unordered_map<std::string, std::string> gecko_container_icons = {
        {"fingerprint", "md_fingerprint"},
        {"briefcase",   "md_work"},
        {"dollar",      "md_attach_money"},
        {"cart",        "md_shopping_cart"},
        {"circle",      "md_circle"},
        {"gift",        "md_redeem"},
        {"vacation",    "md_beach_access"},
        {"food",        "md_local_dining"},
        {"fruit",       "md_restaurant"},
        {"pet",         "md_pets"},
        {"tree",        "md_park"},
        {"chill",       "md_self_improvement"},
        {"fence",       "md_fence"},
    };

    static const std::unordered_map<std::string, ImU32> gecko_container_colors = {
        {"blue",      IM_COL32(0x37, 0xAD, 0xFF, 0xFF)},
        {"turquoise", IM_COL32(0x00, 0xC7, 0x9A, 0xFF)},
        {"green",     IM_COL32(0x51, 0xCD, 0x00, 0xFF)},
        {"yellow",    IM_COL32(0xFF, 0xCB, 0x00, 0xFF)},
        {"orange",    IM_COL32(0xFF, 0x9F, 0x00, 0xFF)},
        {"red",       IM_COL32(0xFF, 0x61, 0x3D, 0xFF)},
        {"pink",      IM_COL32(0xFF, 0x4B, 0xDA, 0xFF)},
        {"purple",    IM_COL32(0xAF, 0x51, 0xF5, 0xFF)},
        {"toolbar",   IM_COL32(0x7C, 0x7C, 0x80, 0xFF)},
    };

    string get_instance_id(const string &reg_value) {
        // if this is Firefox, strip out the prefix to get instance ID
        if(reg_value.starts_with(FirefoxInstancePrefix)) {
            return reg_value.substr(FirefoxInstancePrefix.size());
        }

        return reg_value;
    }

    std::string discovery::unmangle_open_cmd(const std::string &open_cmd) {
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

#if PLATFORM_WINDOWS
    void discovery::discover_win32_registry_browsers(hive h, vector<browser> &browsers,
                                                     const string &ignore_proto) {
        auto subs = enum_subkeys(h, abs_root);

        for(const string &sub: subs) {
            string root = abs_root + "\\" + sub;
            string display_name = get_value(h, root);
            string open_command = get_value(h, root + "\\shell\\open\\command");
            open_command = unmangle_open_cmd(open_command);
            string http_url_assoc = get_value(h,
                                              root + "\\Capabilities\\URLAssociations", "http");

            if(!http_url_assoc.empty() && http_url_assoc != ignore_proto) {
                browser b(display_name, open_command);
                b.instance_id = get_instance_id(sub);

                fingerprint(open_command, b.engine, b.data_path);

                // check for duplicates (HKLM & HKCU can have the same browser registered)
                // this is possible to operator== on browser class

                bool is_dupe{false};
                for(auto &bb: browsers) {
                    if(bb == b) {
                        is_dupe = true;
                        break;
                    }
                }
                if(!is_dupe) {
                    browsers.push_back(std::move(b));
                }
            }
        }
    }
#endif

#if PLATFORM_LINUX
    std::string discovery::resolve_xdg_icon_path(const std::string &icon) {
        if(icon.empty()) {
            return {};
        }

        // Absolute path — use directly
        if(icon[0] == '/') {
            std::error_code ec;
            return fs::exists(icon, ec) ? icon : std::string{};
        }

        // Share dirs to search, in priority order
        std::vector<fs::path> share_dirs;
        if(const char *h = std::getenv("HOME")) {
            share_dirs.emplace_back(fs::path(h) / ".local/share");
            share_dirs.emplace_back(fs::path(h) / ".local/share/flatpak/exports/share");
        }
        if(const char *xdg = std::getenv("XDG_DATA_DIRS")) {
            for(std::string_view sv = xdg; !sv.empty();) {
                auto end = sv.find(':');
                share_dirs.emplace_back(sv.substr(0, end));
                sv.remove_prefix(end == sv.npos ? sv.size() : end + 1);
            }
        } else {
            share_dirs.emplace_back("/usr/share");
            share_dirs.emplace_back("/usr/local/share");
        }
        share_dirs.emplace_back("/var/lib/flatpak/exports/share");
        share_dirs.emplace_back("/var/lib/snapd/desktop");

        const char *exts[] = {".png", ".svg", ".xpm"};

        for(const auto &share: share_dirs) {
            // Enumerate all size dirs under hicolor, sorted largest-first
            fs::path hicolor = share / "icons/hicolor";
            std::error_code ec;

            std::vector<std::pair<int, fs::path> > size_dirs; // {size, apps_path}
            for(const auto &e: fs::directory_iterator(hicolor, ec)) {
                if(!fs::is_directory(e.path(), ec)) continue;
                std::string name = e.path().filename().string();
                int size = 0;
                if(name != "scalable") {
                    auto x = name.find('x');
                    if(x == std::string::npos) continue;
                    try { size = std::stoi(name.substr(0, x)); } catch(...) { continue; }
                }
                size_dirs.emplace_back(size, e.path() / "apps");
            }
            std::sort(size_dirs.begin(), size_dirs.end(),
                      [](const auto &a, const auto &b) { return a.first > b.first; });

            for(const auto &[sz, apps]: size_dirs) {
                for(const char *ext: exts) {
                    fs::path p = apps / (icon + ext);
                    if(fs::exists(p, ec)) {
                        return p.string();
                    }
                }
            }

            // Fallback: pixmaps
            for(const char *ext: exts) {
                fs::path p = share / "pixmaps" / (icon + ext);
                if(fs::exists(p, ec)) {
                    return p.string();
                }
            }
            // Some packages drop the file in pixmaps with no extension
            {
                fs::path p = share / "pixmaps" / icon;
                if(fs::exists(p, ec)) {
                    return p.string();
                }
            }
        }

        return {};
    }

    void discovery::discover_xdg_desktop_browsers(std::vector<browser> &browsers) {
        const char *home = std::getenv("HOME");

        fs::path dirs[] = {
            home ? fs::path(home) / ".local/share/applications" : fs::path{},
            "/usr/share/applications",
            "/usr/local/share/applications",
            "/var/lib/flatpak/exports/share/applications",
            "/var/lib/snapd/desktop/applications",
        };

        std::unordered_set<std::string> seen_exec;

        for(const auto &dir: dirs) {
            std::error_code ec;
            if(!fs::is_directory(dir, ec)) continue;

            for(const auto &de: fs::directory_iterator(dir, ec)) {
                if(de.path().extension() != ".desktop") continue;

                std::string name, exec, categories, mimetypes, icon;

                std::ifstream f(de.path());
                bool in_entry = false;
                std::string line;
                while(std::getline(f, line)) {
                    if(!line.empty() && line.back() == '\r')
                        line.pop_back();

                    if(line == "[Desktop Entry]") {
                        in_entry = true;
                        continue;
                    }

                    if(in_entry && line.size() && line[0] == '[')
                        break; // next section

                    if(!in_entry)
                        continue;

                    auto eq = line.find('=');
                    if(eq == std::string::npos)
                        continue;

                    auto k = line.substr(0, eq);
                    auto v = line.substr(eq + 1);

                    if(k == "Name")
                        name = v;
                    else if(k == "Exec")
                        exec = v;
                    else if(k == "Categories")
                        categories = v;
                    else if(k == "MimeType")
                        mimetypes = v;
                    else if(k == "Icon")
                        icon = v;
                }

                // filter: must be a browser
                auto has_token = [](const std::string &s, std::string_view tok) {
                    for(size_t p = 0; p <= s.size();) {
                        auto e = s.find(';', p);
                        if(e == std::string::npos) e = s.size();
                        if(std::string_view(s).substr(p, e - p) == tok)
                            return true;
                        p = e + 1;
                    }
                    return false;
                };

                if(!has_token(categories, "WebBrowser") && !has_token(mimetypes, "x-scheme-handler/http"))
                    continue;

                if(exec.empty() || !seen_exec.emplace(exec).second)
                    continue;

                // strip field codes (%u, %U, %f, %F, %i, %c, %k, ...)
                std::string cmd;
                for(size_t i = 0; i < exec.size(); ++i) {
                    if(exec[i] == '%' && i + 1 < exec.size()) {
                        ++i;
                        continue;
                    }

                    cmd += exec[i];
                }

                while(!cmd.empty() && cmd.back() == ' ') {
                    cmd.pop_back();
                }

                browser b{name, cmd};
                b.icon_path = resolve_xdg_icon_path(icon);
                fingerprint(cmd, b.engine, b.data_path);
                browsers.push_back(b);
            }
        }
    }
#endif


    std::vector<browser> discovery::discover_browsers(const std::string &ignore_proto) {
        vector<browser> browsers;

#if PLATFORM_WINDOWS
        discover_win32_registry_browsers(hive::local_machine, browsers, ignore_proto);
        discover_win32_registry_browsers(hive::current_user, browsers, ignore_proto);
#endif

#if PLATFORM_LINUX
        discover_xdg_desktop_browsers(browsers);

#endif

        // mark these as fully managed
        for(browser &b: browsers) {
            b.management = management_extent::full;
        }

        discover_managed_profiles(browsers);

        return browsers;
    }

    void discovery::discover_chrome_profiles(browser &b) {
        if(b.engine != browser_engine::chromium) return;

        // https://github.com/ScoopInstaller/Extras/blob/5d9773cbeb8cbe7b1e97061cf4819b60956a3b61/bucket/helium.json#L22

        fs::path root{b.data_path};
        fs::path lsjf = root / "Local State";

        // faster method to discover
        if(fs::exists(lsjf)) {
#if PLATFORM_WINDOWS
            string jt = fss::read_all_text(str::to_str(lsjf));
#else
            string jt = fss::read_all_text(lsjf);
#endif
            auto j = json::parse(jt);
            auto j_p_ic = j["profile"]["info_cache"];
            if(j_p_ic.is_object()) {
                for(auto &jp: j_p_ic.items()) {
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
                    string arg = format(R"("{}" "--profile-directory={}" "--user-data-dir={}" --no-default-browser-check)",
                                        browser::URL_ARG_NAME, sys_name,
                                        b.data_path);

                    browser_profile profile{name, arg,""};
                    if(profile_pic_j.is_string()) {
                        profile.icon_path = (root / sys_name / profile_pic_j.get<string>()).string();
                        if(!fs::is_regular_file(profile.icon_path)) profile.icon_path.clear();
                    }
                    b.profiles.push_back(profile);
                }
            }
        }

        {
            if(b.open_cmd.find("msedge.exe") != string::npos) {
                // Edge is not stupid, it's just different
                browser_profile inprivate(
                    "InPrivate",
                    format("\"{}\" --inprivate", browser::URL_ARG_NAME),
                    "");
                inprivate.is_incognito = true;
                b.profiles.push_back(inprivate);
            } else {
                browser_profile inprivate(
                    "Incognito",
                    format("\"{}\" --incognito", browser::URL_ARG_NAME),
                    "");
                inprivate.is_incognito = true;
                b.profiles.push_back(inprivate);
            }
        }

        // Brave additionally supports Tor mode
        if(b.name == "brave") {
            browser_profile tor(
                "Tor",
                format("\"{}\" --tor", browser::URL_ARG_NAME),
                ""
            );
            tor.is_incognito = true;
            b.profiles.push_back(tor);
        }
    }

    void discovery::discover_gecko_profile_groups(
        const string &parent_id,
        const string &installation_id,
        const string &store_id,
        const string &sqlite_db_path,
        const string &data_folder_path,
        std::vector<firefox_profile> &profiles) {
        // select * from Profiles
        sqlite3 *db;
        int rc = sqlite3_open(sqlite_db_path.c_str(), &db);
        if(rc == SQLITE_OK) {
            auto sql_profiles = sql_execute(db, "select * from Profiles");
            for(const auto &profile: sql_profiles) {
                auto it_id = profile.find("id");
                auto it_name = profile.find("name");
                auto it_path = profile.find("path");

                string id = it_id != profile.end() ? it_id->second : "";
                string full_id = format("s{}i{}", store_id, id);
                string name = it_name != profile.end() ? it_name->second : full_id;
                string path = it_path != profile.end() ? it_path->second : "";
                path = (fs::path{data_folder_path} / path).string();

                profiles.emplace_back(full_id, parent_id, installation_id, name, path, false);
            }
        }
        sqlite3_close(db);
    }


    void discovery::discover_gecko_profiles(browser &b, std::vector<firefox_profile> &profiles) {
        fs::path data_folder{b.data_path};

        // profiles.ini is the starting entry point to find both classic and new profiles (profile groups)
        fs::path ini_path = data_folder / "profiles.ini";
        if(!fs::exists(ini_path)) return;

        CSimpleIniA ini;
        ini.LoadFile(ini_path.c_str());
        list<CSimpleIniA::Entry> ir;
        ini.GetAllSections(ir);

        // create a map of profile id to installation id
        map<string, string> profile_to_installation_id;
        for(CSimpleIni::Entry &section: ir) {
            string section_name = section.pItem;
            if(!section_name.starts_with("Install")) continue;

            const char *c_profile_path = ini.GetValue(section.pItem, "Default");
            if(c_profile_path == nullptr) continue;

            string profile_path = c_profile_path;
            string installation_id = section_name.substr(strlen("Install"));

            if(!profile_path.empty() && !installation_id.empty()) {
                profile_to_installation_id[profile_path] = installation_id;
            }
        }

        // extract all the profiles
        for(CSimpleIniA::Entry &e: ir) {
            // a section is a profile if starts with "Profile".
            string section_name{e.pItem};
            if(!section_name.starts_with("Profile")) continue;

            // extract display name if possible
            const char *c_name = ini.GetValue(e.pItem, "Name");
            string display_name = c_name ? c_name : section_name;

            // if is_relative is false, this is an absolute path (not like it matters anyway)
            const char *c_is_relative = ini.GetValue(e.pItem, "IsRelative");
            const char *c_path = ini.GetValue(e.pItem, "Path");
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
            const char *c_nested_store_id = ini.GetValue(e.pItem, "StoreID");
            if(c_nested_store_id) {
                fs::path sqlite_db_path = data_folder / "Profile Groups" / (string{c_nested_store_id} + ".sqlite");
                // profile definitions i.e. "new profiles" are now stored in the sqlite database
                if(fs::exists(sqlite_db_path)) {
                    discover_gecko_profile_groups(section_name,
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

    void discovery::discover_gecko_profiles(browser &b) {
        if(b.engine != browser_engine::gecko) return;

        vector<firefox_profile> profiles;
        discover_gecko_profiles(b, profiles);

        // sort profiles using the following rules: is_classic, has installation_id, name
        std::sort(profiles.begin(), profiles.end(),
                  [](const firefox_profile &a, const firefox_profile &b) {
                      if(a.is_classic != b.is_classic) {
                          return !a.is_classic; // classic profiles last
                      }
                      if((!a.installation_id.empty()) != (!b.installation_id.empty())) {
                          return !a.installation_id.empty(); // profiles with installation_id first
                      }
                      return a.name < b.name; // finally by name
                  });

        for(firefox_profile &fp: profiles) {
            // if profile is bound to an installation, but it's not ours, skip it always
            if(!fp.installation_id.empty() && fp.installation_id != b.instance_id) continue;

            if(fp.installation_id.empty() && !g_state.discover_classic_gecko_profiles) continue;

            string arg_suffix = fp.is_classic
                                    ? format("-P \"{}\"", fp.name)
                                    : format(R"("--profile" "{}")", fp.path);
            string arg = format("\"{}\" -foreground {}", browser::URL_ARG_NAME, arg_suffix);

            browser_profile p(fp.name, arg, "");
            b.profiles.push_back(p);

            // containers
            if(g_state.discover_gecko_containers) {
                // for each container, add a profile
                // Leave the "no container" profile as is.

                // add profile for each container
                vector<firefox_container> containers = discover_gecko_containers(fp.path);
                for(const auto &container: containers) {
                    string arg = format("\"ext+container:name={}&url={}\" {}",
                                        container.name,
                                        browser::URL_ARG_NAME,
                                        arg_suffix);

                    browser_profile bi(container.name, arg, "");
                    if(container.has_color) {
                        bi.use_color = true;
                        bi.color = container.color;
                    }
                    b.profiles.push_back(bi);
                }
            }
        }

        // Safe Mode starts FF without extensions
        //auto safe_bi = browser_profile(b, "safemode", "Safe Mode",
        //   arg + " -safe-mode",
        //   b.open_cmd);

        // in-private
        browser_profile private_bi("Private",
                                   format("-private-window \"{}\"", browser::URL_ARG_NAME), "");
        private_bi.is_incognito = true;

        b.profiles.push_back(private_bi);
    }

    vector<firefox_container> discovery::discover_gecko_containers(const string &roaming_home) {
        vector<firefox_container> r;

        // detect if "containers" are installed
        fs::path containers_path = fs::path{roaming_home} / "containers.json";
        if(fs::exists(containers_path)) {
            string jt = fss::read_all_text(containers_path.string());
            auto j = json::parse(jt);
            auto identities = j["identities"];
            if(identities.is_array()) {
                for(json::iterator it = identities.begin(); it != identities.end(); ++it) {
                    /* an example of identity element:
{
  "userContextId": 9,
  "public": true,
  "icon": "tree",
  "color": "green",
  "name": "Braden"
},
                     */


                    auto identity = *it;
                    auto j_is_public = identity["public"];
                    if(!j_is_public.is_boolean() || !j_is_public.get<bool>()) continue;

                    auto j_id = identity["userContextId"];
                    if(!j_id.is_number()) continue;

                    auto j_name = identity["name"];
                    auto j_l10nID = identity["l10nID"];
                    auto j_l10nId = identity["l10nId"];
                    auto j_icon = identity["icon"];
                    auto j_color = identity["color"];
                    //string t0 = j_name.type_name();
                    //string t1 = j_l10nID.type_name();

                    if(!(j_name.is_string() || j_l10nID.is_string() || j_l10nId.is_string())) continue;

                    int id = j_id.get<int>();
                    string name;
                    if(j_name.is_string()) {
                        name = j_name.get<string>();
                    } else {
                        // there are 4 default containers - Personal, Work, Banking, Shopping.
                        // They can be deleted or renamed (in which case they will get the "name" property set)

                        string lid = j_l10nID.is_string() ? j_l10nID.get<string>() : j_l10nId.get<string>();

                        auto it = gecko_container_names.find(lid);
                        if(it != gecko_container_names.end()) {
                            name = it->second;
                        } else {
                            name = lid;
                        }
                    }

                    string icon_name;
                    string color_name;
                    if(j_icon.is_string()) icon_name = j_icon.get<string>();
                    if(j_color.is_string()) color_name = j_color.get<string>();

                    string icon;
                    if(gecko_container_icons.contains(icon_name)) {
                        icon = gecko_container_icons.at(icon_name);
                    }
                    bool has_color{false};
                    unsigned int color{0};
                    if(gecko_container_colors.contains(color_name)) {
                        has_color = true;
                        color = gecko_container_colors.at(color_name);
                    }

                    r.emplace_back(to_string(id), name, icon, has_color, color);
                }
            }
        }

        return r;
    }

    std::vector<std::string> discovery::get_firefox_addons_installed(const std::string &roaming_home) {
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

    void discovery::discover_other_profiles(browser &b) {
        if(b.engine != browser_engine::generic) return;

        string icon_path = b.icon_path.empty() ? b.open_cmd : b.icon_path;

        const browser_profile bi("Default",
                                 format("\"{}\"", browser::URL_ARG_NAME),
                                 icon_path);

        b.profiles.push_back(bi);
    }

    std::vector<browser> discovery::discover_all_browsers() {
        return discover_browsers(ProtoName);
    }

    void discovery::discover_managed_profiles(std::vector<browser> &browsers) {
        // discover various profiles
        for(browser &b: browsers) {
            discover_managed_profiles(b);
        }
    }

    void discovery::discover_managed_profiles(browser &b) {
        if(b.management == management_extent::full || b.management == management_extent::profiles) {
            discover_chrome_profiles(b);
            discover_gecko_profiles(b);
            discover_other_profiles(b);
        }
    }

    bool discovery::fingerprint(const std::string &exe_path, browser_engine &engine, std::string &data_path) {
        engine = browser_engine::generic;
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
        for(const auto &entry: fs::directory_iterator(folder_path)) {
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
        for(const auto &entry: fs::directory_iterator(folder_path)) {
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
