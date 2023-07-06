#include "discovery.h"
#include "fss.h"
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

        fs::path root(lad);
        root.append(b->vdf);

        // faster method to discover
        fs::path lsjf = fs::path{lad} / b->vdf / "Local State";
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
                    bi->home_path = str::to_str(fs::path{lad} / b->vdf / sys_name);
                    if(profile_pic_j.is_string()) {
                        bi->icon_path = str::to_str(fs::path{lad} / b->vdf / sys_name / profile_pic_j.get<string>());
                    }
                    b->instances.push_back(bi);
                }
            }
        }

        {
            auto inprivate = make_shared<browser_instance>(
                b, "InPrivate", "Private",
                fmt::format("\"{}\" -inprivate", browser_instance::URL_ARG_NAME),
                ""
            );
            inprivate->is_incognito = true;
            b->instances.push_back(inprivate);
        }

        // Brave additionally supports Tor mode
        if(b->id == "brave") {
            auto tor = make_shared<browser_instance>(
                b, "tor", "Tor",
                fmt::format("\"{}\" --tor", browser_instance::URL_ARG_NAME),
                ""
            );
            tor->is_incognito = true;
            b->instances.push_back(tor);
        }
    }

    void discovery::discover_firefox_profiles(shared_ptr<browser> b) {
        if (!(b->is_system && b->is_firefox)) return;

        {
            // see http://kb.mozillazine.org/Profiles.ini_file
            fs::path ini_path = fs::path{ad} / "Mozilla" / "Firefox" / "profiles.ini";
            if(fs::exists(ini_path)) {
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

                    const char* c_default = ini.GetValue(e.pItem, "Default");
                    bool is_default = c_default != nullptr && string{c_default} == "1";

                    string arg = fmt::format("\"{}\" -P \"{}\"", browser_instance::URL_ARG_NAME, name);
                    auto bi = make_shared<browser_instance>(
                        b, e.pItem, name, arg, b->open_cmd
                    );
                    bi->home_path = is_relative
                        ? (fs::path{lad} / "Mozilla" / "Firefox" / c_path).string()
                        : c_path;

                    // detect if "containers" are installed
                    fs::path containers_path = fs::path{ad} / "Mozilla" / "Firefox" / c_path / "containers.json";
                    if(fs::exists(containers_path)) {
                        bi->firefox_containers_config_path = containers_path.string();
                    }

                    b->instances.push_back(bi);
                }
            }
        }

        // Safe Mode starts FF without extensions
        //auto safe_bi = browser_instance(b, "safemode", "Safe Mode",
        //   arg + " -safe-mode",
        //   b.open_cmd);

        //result.push_back(safe_bi);

        // in-private

        auto private_bi = make_shared<browser_instance>(b, "private", "Private",
            fmt::format("-private-window \"{}\"", browser_instance::URL_ARG_NAME),
            b->open_cmd);
        private_bi->is_incognito = true;

        b->instances.push_back(private_bi);
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