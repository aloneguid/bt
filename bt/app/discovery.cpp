#include "discovery.h"
#include "fss.h"
#include "config.h"
#include "glob.hpp"
#include "str.h"
#include "win32/shell.h"
#include "win32/reg.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <fmt/core.h>
#include "../globals.h"
#include <SimpleIni.h> // https://github.com/brofield/simpleini

using namespace std;
using hive = win32::reg::hive;
namespace fs = std::filesystem;
using json = nlohmann::json;

// mapping of vendors to vendor data folder

map<string, string> chromium_id_to_vdf {
    { "msedge", "Microsoft\\Edge\\User Data" },
    { "chrome", "Google\\Chrome\\User Data" },
    { "vivaldi", "Vivaldi\\User Data" },
    { "brave", "BraveSoftware\\Brave-Browser\\User Data" },
    { "thorium", "Thorium\\User Data" }
};

map<string, string> firefox_id_to_vdf{
    { "firefox", "Mozilla\\Firefox" },
    { "waterfox", "Waterfox" }
};

namespace bt {
    const string abs_root = "SOFTWARE\\Clients\\StartMenuInternet";
    const string ad = win32::shell::get_app_data_folder();
    const string lad = win32::shell::get_local_app_data_path();

    string get_id_from_open_cmd(const std::string& cmd) {
        fs::path p(cmd);
        p.replace_extension();
        return p.filename().string();
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

        if(!chromium_id_to_vdf.contains(b->id)) return;

        fs::path root = fs::path{lad} / chromium_id_to_vdf[b->id];
        fs::path lsjf = root / "Local State";

        // faster method to discover
        if(fs::exists(lsjf)) {
            string jt = fss::read_file_as_string(str::to_str(lsjf));
            auto j = json::parse(jt);
            auto j_p_ic = j["profile"]["info_cache"];
            if(j_p_ic.is_object()) {
                for(auto& jp : j_p_ic.items()) {
                    string sys_name = jp.key();
                    auto name_j = jp.value()["shortcut_name"];
                    auto profile_pic_j = jp.value()["gaia_picture_file_name"];

                    string name = name_j.is_string() ? name_j.get<string>() : "";

                    // all the data is ready
                    string arg = fmt::format("\"{}\" \"--profile-directory={}\"",
                        browser_instance::URL_ARG_NAME, sys_name);

                    auto bi = make_shared<browser_instance>(
                        b,
                        sys_name,
                        name,
                        arg,
                        ""
                    );
                    bi->order = b->instances.size();
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
                inprivate->order = b->instances.size();
                b->instances.push_back(inprivate);
            } else {
                auto inprivate = make_shared<browser_instance>(
                b, "Incognito", "Incognito",
                fmt::format("\"{}\" --incognito", browser_instance::URL_ARG_NAME),
                "");
                inprivate->is_incognito = true;
                inprivate->order = b->instances.size();
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
            tor->order = b->instances.size();
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
                firefox_container_mode container_mode = g_config.get_firefox_container_mode();

                CSimpleIniA ini;
                ini.LoadFile(ini_path.c_str());
                list<CSimpleIniA::Entry> ir;
                ini.GetAllSections(ir);
                for(auto& e : ir) {
                    // a section is potentially a profile
                    const char* c_name = ini.GetValue(e.pItem, "Name");
                    if(!c_name) continue;
                    string name = c_name;

                    // if is_relative is false, this is an absolute path (not like it matters anyway)
                    const char* c_is_relative = ini.GetValue(e.pItem, "IsRelative");
                    bool is_relative = c_is_relative == nullptr || string{c_is_relative} == "1";
                    const char* c_path = ini.GetValue(e.pItem, "Path");
                    if(!c_path) continue;

                    // We don't want "default" profile - in Firefox it's some kind of pre-release oddness.
                    // In Waterfox it's called something like "68-edition-default"
                    if(name == "default" || name.ends_with("-edition-default")) continue;

                    string local_home = is_relative
                        ? (fs::path{lad} / vdf / c_path).string()
                        : c_path;

                    string roaming_home = is_relative
                        ? (fs::path{ad} / vdf / c_path).string()
                        : c_path;


                    if(container_mode == firefox_container_mode::off) {

                        // rename primary release profile to something more human-readable
                        string profile_display_name = name == "default-release"
                            ? "Primary"
                            : name;

                        string arg = fmt::format("\"{}\" -P \"{}\"", browser_instance::URL_ARG_NAME, name);
                        auto bi = make_shared<browser_instance>(b, e.pItem, profile_display_name, arg, "");
                        bi->order = b->instances.size();

                        {
                            const char* c_default = ini.GetValue(e.pItem, "Default");
                            bi->is_default = c_default != nullptr && string{c_default} == "1";
                        }

                        b->instances.push_back(bi);
                    } else {

                        /*vector<string> addons = get_firefox_addons_installed(roaming_home);
                        if(std::find(addons.begin(), addons.end(), "{f069aec0-43c5-4bbf-b6b4-df95c4326b98}") != addons.end()) {
                            bi->has_firefox_ouic_addon = true;
                        }*/

                        vector<firefox_container> containers = discover_firefox_containers(roaming_home);
                        for(const auto& container : containers) {

                            string arg;

                            if(container_mode == firefox_container_mode::ouic) {
                                arg = fmt::format("\"ext+container:name={}&url={}\" -P \"{}\"",
                                    container.name,
                                    browser_instance::URL_ARG_NAME,
                                    name);
                            } else if(container_mode == firefox_container_mode::bt) {
                                arg = fmt::format("\"ext+bt:container={}&url={}\" -P \"{}\"",
                                    container.name,
                                    browser_instance::URL_ARG_NAME,
                                    name);
                            } else {
                                arg = fmt::format("unknown mode", config::firefox_container_mode_to_string(container_mode));
                            }

                            string id = fmt::format("{}+c_{}", e.pItem, container.id);
                            auto bi = make_shared<browser_instance>(b, id, container.name, arg, "");
                            bi->order = b->instances.size();
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
        private_bi->order = b->instances.size();

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
                    if(!(j_name.is_string() || j_l10nID.is_string())) continue;

                    int id = j_id.get<int>();
                    string name;
                    if(j_name.is_string()) {
                        name = j_name.get<string>();
                    } else {
                        // there are 4 default containers - Personal, Work, Banking, Shopping.
                        // They can be deleted, or renamed (in which case they will get "name" property set)

                        string lid = j_l10nID.get<string>();
                        if(lid == "userContextPersonal.label") {
                            name = "Personal";
                        } else if(lid == "userContextWork.label") {
                            name = "Work";
                        } else if(lid == "userContextBanking.label") {
                            name = "Banking";
                        } else if(lid == "userContextShopping.label") {
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

        if (!(b->is_system && b->instances.empty())) return;

        string arg("\"");
        arg += browser_instance::URL_ARG_NAME;
        arg += "\"";

        auto bi = make_shared<browser_instance>(b, "default", "Default",
            arg,
            b->open_cmd);

        b->instances.push_back(bi);
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
        return chromium_id_to_vdf.find(system_id) != chromium_id_to_vdf.end();
    }

    bool discovery::is_firefox_id(const std::string& system_id) {
        return firefox_id_to_vdf.find(system_id) != firefox_id_to_vdf.end();
    }

    string discovery::get_shell_url_association_progid(const string& protocol_name) {
        string prog_id = win32::reg::get_value(
            win32::reg::hive::current_user,
            fmt::format("Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\{}\\UserChoice", protocol_name),
            "ProgId");

        return prog_id;
    }

    string get_settings_root() {
        return string("SOFTWARE\\") + APP_LONG_NAME;
    }
}