#include "configure.h"
#include "../../res.inl"
#include "fmt/core.h"
#include "win32/process.h"
#include "win32/shell.h"
#include "win32/sysinfo.h"
#include "win32/user.h"
#include "win32/ole32.h"
#include "win32/clipboard.h"
#include "str.h"
#include "stl.hpp"
#include "../rule_hit_log.h"
#include <filesystem>
#include "../../globals.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    config_app::config_app() {
        title = string{APP_LONG_NAME} + " " + APP_VERSION;
        browsers = bt::browser::get_cache();
        app = grey::app::make(title);
        float padding_bottom = 20 * app->scale;
        w_left_panel = w::container{250 * app->scale, -padding_bottom};
        w_right_panel = w::container{0, -padding_bottom}.border();

        check_health();
    }

    void config_app::run() {
        app->run([this](const grey::app& app) { return run_frame(); });
    }

    void config_app::check_health() {
        health_checks = setup::get_checks();

        health_succeeded = health_failed = 0;

        for(auto& sc : health_checks) {
            sc.recheck();

            if(sc.is_ok) {
                health_succeeded += 1;
            } else {
                health_failed += 1;
            }
        }
    }

    bool config_app::run_frame() {
        w::window wnd{title, &is_open};
        wnd
            .size(900, 450, app->scale)
            .has_menubar()
            .no_resize()
            .render();

        {
            w::menu_bar menu{menu_items, [this](const string& id) { handle_menu_click(id); }};
        }

        if(browsers.empty()) {
            render_no_browsers();
        } else {
            render_browsers();
        }

#if _DEBUG
        if(show_demo) {
            ImGui::ShowDemoWindow();
        }
#endif

        //render_status_bar();

        if(show_about)
            render_about_window();

        if(show_dash)
            render_dashboard_window();

        return is_open;
    }

    void config_app::handle_menu_click(const std::string& id) {
        if(id == "+b") {
            add_custom_browser_by_asking();
        } else if(id == "ini") {
            win32::shell::exec(g_config.get_absolute_path(), "");
        } else if(id == "ini+c") {
            win32::clipboard::set_ascii_text(g_config.get_absolute_path());
        } else if(id == "csv") {
            win32::shell::exec(rule_hit_log::i.get_absolute_path(), "");
        } else if(id == "csv+c") {
            win32::clipboard::set_ascii_text(rule_hit_log::i.get_absolute_path());
        } else if(id == "demo") {
            show_demo = !show_demo;
        } else if(id.starts_with("set_theme")) {
            grey::themes::set_theme(id, app->scale);
        } else if(id == "?") {
            show_about = !show_about;
        } else if(id == "dash") {
            show_dash = !show_dash;
        } else if(id == "x") {
            is_open = false;
        }
    }

    void config_app::render_about_window() {
        const int width = 310;
        app->preload_texture("logo", icon_png, icon_png_len);
        w::window wnd{"About"};
        wnd
            .size(width, 355, app->scale)
            .no_resize()
            .render();

        float icon_size = 50 * app->scale;
        w::set_pos(width * app->scale / 2 - icon_size / 2, 40 * app->scale);
        w::image(*app, "logo", icon_size, icon_size);
        w::spc(3);

        w::label(
            R"(Browser Tamer acts as a virtual browser, but instead of showing web pages, it redirects links to a browser or browser profile of your choice.

It super fast, extremely light on resources, completely free and open source.)",
300 * app->scale);
        w::spc();

        w::label("Version:");

        w::sl(); w::label(APP_VERSION, w::emphasis::primary);

        w::label("ImGui Version:");
        w::sl(); w::label(ImGui::GetVersion(), w::emphasis::primary);

        // sys info

        // refresh sys info
        {
            about_frame_time += ImGui::GetIO().DeltaTime;
            if(about_frame_time >= 1) {
                about_frame_time = 0;

                about_fps = fmt::format("{} {:.1f}", ICON_MD_SCREENSHOT_MONITOR, ImGui::GetIO().Framerate);
                about_fps_tooltip = fmt::format(
                    "Framerate: {:.1f}\nScale: {:.1f}\nDPI: {}",
                    ImGui::GetIO().Framerate,
                    app->scale,
                    win32::shell::get_dpi());

                // memory usage
                win32::process p;
                uint64_t working_set;
                if(p.get_memory_info(working_set)) {
                    about_mem = fmt::format("{} {}", ICON_MD_MEMORY, str::to_human_readable_size(working_set));
                }

                // total CPU usage
                about_cpu = fmt::format("{} {:.1f}", ICON_MD_DEVELOPER_BOARD, win32::system_info::get_cpu_usage_perc());
            }
        }

        w::spc();

        w::label(about_fps);
        w::tooltip(about_fps_tooltip);

        w::label(about_mem);
        w::tooltip("Process memory usage (when this dialog is closed, usage goes down massively)");

        w::label(about_cpu);
        w::tooltip("Current CPU load of your entire system (not this application) in percentages.");

        w::sep();
        w::spc();

        if(w::button(ICON_MD_HOME " Home")) {
            // ui::url_open(url_payload{APP_URL}, ui::open_method::configured);
        }
        w::sl();
        if(w::button("GitHub")) {
            //ui::url_open(url_payload{APP_GITHUB_URL}, ui::open_method::configured);
        };
        w::sl();
        if(w::button("Close")) {
            show_about = false;
        }
    }

    void config_app::render_dashboard_window() {
        w::window wnd{"Readiness Dashboard"};
        wnd
            .size(400, 160, app->scale)
            .no_resize()
            .render();


        for(auto& sc : health_checks) {
            string tooltip = sc.description;
            {
                w::group g;
                g.render();

                w::label(sc.is_ok ? ICON_MD_DONE : ICON_MD_ERROR, sc.is_ok ? w::emphasis::primary : w::emphasis::error);
                w::sl();
                w::label(sc.name);

                if(!sc.is_ok) {
                    w::sl();
                    if(w::button("fix", w::emphasis::error)) {
                        sc.fix();
                        check_health();
                    }
                }
            }
            w::tooltip(tooltip);
        }

        w::spc();

        if(health_failed == 0) {
            w::label("all checks succeeded");
        } else {
            w::label(fmt::format("{} checks failed out of {}", health_failed, health_succeeded + health_failed));
        }

        w::sep();
        w::spc();

        if(w::button("recheck", w::emphasis::primary)) {
            check_health();
        }
        w::sl();
        if(w::button("close")) {
            show_dash = false;
        }
    }
    
    void config_app::render_status_bar() {
        w::status_bar sb;

        w::label(ICON_MD_HEALTH_AND_SAFETY);
        if(w::is_leftclicked()) {
            //bool healthy = w_dash->recheck();
            //if(!healthy && !dash_visible) dash_visible = true;
        }

        size_t ipc{0};
        size_t irc{0};
        for(const auto& b : browsers) {
            ipc += b->instances.size();
            for(const auto& i : b->instances) {
                irc += i->rules.size();
            }
        }

        w::label("|", 0, false);
        w::label(fmt::format("{} {}", ICON_MD_WEB, browsers.size()), 0, false);
        w::tooltip("Browser count");

        w::label(fmt::format("{} {}", ICON_MD_PERSON, ipc), 0, false);
        w::tooltip("Profile count");

        w::label(fmt::format("{} {}", ICON_MD_RULE, irc), 0, false);
        w::tooltip("Configured rule count");


        w::label("|", 0, false);

        w::label(ICON_MD_COFFEE, 0, false);
        w::tooltip("Support this app, buy me a coffee!");
        if(w::is_leftclicked()) {
            //ui::url_open(url_payload{APP_BUYMEACOFFEE_URL}, ui::open_method::configured);
        };
    }

    void config_app::render_no_browsers() {
        for(int i = 0; i < 5; i++)
            w::label("");

        w::label(string(90, ' ')); w::sl();
        w::label("Currently there are no browsers registered.", w::emphasis::primary);

        w::label(string(70, ' ')); w::sl();
        w::label("Press the button below to scan your system for installed browsers.");
        w::label("");
        w::label(string(100, ' ')); w::sl();
        if(w::button("discover system browsers", w::emphasis::error)) {
            //rediscover_browsers();
        }
    }

    void config_app::render_browsers() {
        {
            w::guard g{w_left_panel};

            if(w::button(ICON_MD_ADD_CIRCLE " Add", w::emphasis::primary)) {
                add_custom_browser_by_asking();
            }
            w::tooltip("Add custom browser definition");
            w::sl();
            show_hidden_browsers = w::icon_checkbox(ICON_MD_VISIBILITY, show_hidden_browsers);
            w::tooltip("Show hidden browsers");

            for(int i = 0; i < browsers.size(); i++) {
                auto br = browsers[i];
                if(!show_hidden_browsers && br->is_hidden) {
                    continue;
                }

                render_card(br, i == selected_browser_idx);

                // we can now react on click
                if(ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    selected_browser_idx = i;
                }
            }
        }
        w::sl();
        {
            w::guard g{w_right_panel};

            if(selected_browser_idx < browsers.size()) {
                render_detail(browsers[selected_browser_idx]);
            }
        }
    }

    void config_app::render_card(std::shared_ptr<bt::browser> b, bool is_selected) {

        w::group g;
        g
            .border_hover(ImGuiCol_HeaderActive)
            .spread_horizontally();

        if(is_selected) {
            g.border(ImGuiCol_HeaderHovered);
        }

        g.render();

        float padding = 10 * app->scale;
        float icon_size = 32 * app->scale;
        float left_pad = icon_size + padding * 2;

        // render icon and come back to starting position
        w::set_pos(0, -1);
        w::move_pos(padding, padding);
        if(b->open_cmd.empty() || !std::filesystem::exists(b->open_cmd)) {
            app->preload_texture("logo", icon_png, icon_png_len);
            w::image(*app, "logo", icon_size, icon_size);
        } else {
            app->preload_texture(b->open_cmd, b->open_cmd);
            w::image(*app, b->open_cmd, icon_size, icon_size);
        }
        w::set_pos(0, -1);
        w::move_pos(0, -(icon_size + padding));

        // elements
        w::set_pos(0, -1);
        w::move_pos(left_pad, 0);
        w::label(b->name);

        w::move_pos(left_pad, 0);

        if(b->instances.size() > 0) {
            if(b->is_system) {
                w::label(fmt::format("{} {}", ICON_MD_FACE, b->instances.size()), 0, false);
                w::tooltip(str::humanise(b->instances.size(), "profile", "profiles"));
                //i_where->is_enabled = false;

                if(b->is_chromium) {
                    w::sl();
                    w::label(ICON_MD_TAB, 0, false);
                    w::tooltip("Chromium-based");
                } else if(b->is_firefox) {
                    w::sl();
                    w::label(ICON_MD_WHATSHOT, 0, false);
                    w::tooltip("Firefox-based");
                }
            } else {
                w::label(ICON_MD_SUPPORT_AGENT, 0, false);
                w::tooltip("User-defined");
            }

            if(b->get_supports_frameless_windows()) {
                w::sl();
                w::label(ICON_MD_TAB_UNSELECTED, 0, false);
                w::tooltip("Supports frameless windows");
            }

            if(b->is_hidden) {
                w::sl();
                w::label(ICON_MD_HIDE_IMAGE, 0, false);
                w::tooltip("Hidden");
            }

        } else {
            w::label(ICON_FK_USER, 0, false);
            w::tooltip("no profiles");
        }

        w::spc();
        w::set_pos(0, -1);
    }

    void config_app::render_detail(std::shared_ptr<bt::browser> b) {
        // universal test button
        w::sl();
        if(w::button(ICON_MD_LAUNCH)) {
            //auto instance = b->instances[b->is_system ? profiles_tabs->get_selected_idx() : 0];
            url_payload pl{APP_URL};
            //instance->launch(pl);
        }
        w::tooltip("test by opening a link");

        if(b->is_chromium) {
            w::sl();
            if(w::button(ICON_MD_OPEN_IN_NEW)) {
                //auto instance = b->instances[b->is_system ? profiles_tabs->get_selected_idx() : 0];
                //url_payload pl{APP_URL};
                //pl.app_mode = true;
                //instance->launch(pl);
            }
            w::tooltip("test by opening a link as an app");
        }

        if(b->is_system) {

            w::sl();
            if(!b->open_cmd.empty()) {
                if(w::button(ICON_MD_FOLDER)) {
                    std::filesystem::path p{b->open_cmd};
                    string path = p.parent_path().string();
                    win32::shell::exec(path, "");
                }
                w::tooltip(fmt::format("open {}'s folder in Explorer.", b->name));

                if(b->is_firefox) {

                    /*auto cm = g_config.get_firefox_container_mode();

                    if(cm == firefox_container_mode::off) {
                        g_static->same_line();
                        auto pmgr = g_static->make_button(ICON_FK_ADDRESS_CARD);
                        pmgr->tooltip = "open Firefox Profile Manager (-P flag)";
                        pmgr->on_pressed = [b](button&) {
                            win32::shell::exec(b->open_cmd, "-P");
                        };

                        g_static->same_line();
                        pmgr = g_static->make_button(ICON_FK_ID_CARD);
                        pmgr->tooltip = "open Firefox Profile Manager in Firefox itself";
                        pmgr->on_pressed = [b](button&) {
                            win32::shell::exec(b->open_cmd, "about:profiles");
                        };
                    } else {

                        g_static->same_line();
                        auto cmd_x = g_static->make_button(ICON_FK_PUZZLE_PIECE);
                        cmd_x->tooltip = "download required extension";
                        cmd_x->set_emphasis(emphasis::error);
                        cmd_x->on_pressed = [this, b](button&) {
                            ui::url_open(url_payload{"https://aloneguid.github.io/bt/firefox-containers.html#install-extension"}, bt::ui::open_method::configured);
                        };
                    }*/
                } else if(b->is_chromium) {
                    w::sl();
                    if(w::button(ICON_MD_EXTENSION)) {
                        // extenstion page needs to be opened in the correct profile
                        //auto instance = b->instances[b->is_system ? profiles_tabs->get_selected_idx() : 0];
                        //instance->launch(url_payload{APP_BROWSER_EXTENSIONS_DOCS_URL});
                    }
                    w::tooltip("download optional integration extension");
                }

            }
        } else {
            w::sl();
            if(w::button(ICON_MD_DELETE " delete", w::emphasis::error)) {
                size_t idx = browser::index_of(browsers, b);

                // erase and save
                vector<shared_ptr<browser>> all = browser::get_cache();
                std::erase_if(all, [b](auto i) { return i->id == b->id; });
                g_config.save_browsers(all);
                browsers = browser::get_cache(true); // invalidate

                // if possible, select previous browser
                if(idx != string::npos) {
                    idx -= 1;
                    if(idx >= 0 && idx < browsers.size()) {
                        selected_browser_idx = idx;
                    }
                }
            }
            w::tooltip("Completely deletes this browser, no questions asked");
        }

        // hide/show button
        w::sl();
        b->is_hidden = !w::icon_checkbox(ICON_MD_VISIBILITY, !b->is_hidden);
        w::tooltip("Show or hide this browser from the browser list");

        // --- toolbar end

        // --- profiles start

        if(b->is_system) {
            w::tab_bar tabs{b->id};

            for(shared_ptr<browser_instance> bi : b->instances) {
                string tab_icon;
                if(bi->is_incognito) {
                    tab_icon = fmt::format("{} ", ICON_MD_SECURITY);
                }
                string tab_title = fmt::format(" {}{} ", tab_icon, bi->name);

                {
                    auto t = tabs.next_tab(tab_title);
                    if(t) {
                        w::spc();

                        if(w::accordion("Parameters")) {
                            w::input(bi->launch_arg, "arg", false);
                            w::tooltip("Discovered arguments (read-only)");

                            w::input(bi->user_arg, "extra arg");
                            w::tooltip("Any extra arguments to pass.\nIf you break it, you fix it ;)");
                        }

                        w::spc();
                        render_rules(bi);
                    }
                }
            }
        } else {
            shared_ptr<browser_instance> bi = b->instances[0];

            if(w::accordion("Parameters")) {
                w::input(b->open_cmd, "exe", false);
                w::tooltip("Full path to browser executable. The only way to change this is to re-create the browser. Sorry ;)");

                w::input(bi->name, "name");
                //b->name = bi->name;

                w::input(bi->launch_arg, "arg");
                w::tooltip(R"(Argument(s) to pass to the browser.
It is empty by default and opening url is always passed as an argument.
If you set this value, it is used as is. Also, 'arg' can contain a
special keyword - %url% which is replaced by opening url.)");
            }

            // rules
            w::spc();
            render_rules(bi);
        }
    }

    void config_app::render_rules(std::shared_ptr<browser_instance> bi) {
        w::label("Rules");

        w::button(ICON_MD_ADD " add", w::emphasis::primary);
        w::sl();
        w::button(ICON_MD_DELETE " clear all", w::emphasis::error);
        w::sl();
        w::button(ICON_MD_CRUELTY_FREE " test");
        w::tooltip("Open URL Tester");

        // scrollable area with list of rules
        {
            w::container c{"rules"};
            w::guard g{c};

            for(int i = 0; i < bi->rules.size(); i++) {
                auto rule = bi->rules[i];

                // location
                w::combo(string{"##loc"} + std::to_string(i), 
                    rule_locations, (size_t&)rule->loc, 70 * app->scale);

                // value
                w::sl();
                w::input(rule->value, string{"##val"} + std::to_string(i), true, 250 * app->scale);

                // up/down logic is very custom and is bound to the textbox itself
                if(ImGui::IsItemFocused()) {
                    if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                        stl::move(bi->rules, i, -1, true);
                    } else if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                        stl::move(bi->rules, i, 1, true);
                    }
                }

                // is regex checkbox
                w::sl();
                rule->is_regex = w::icon_checkbox(ICON_MD_GRAIN, rule->is_regex);
                w::tooltip("Rule is a Regular Expression (advanced)");

                // app mode
                if(bi->b->is_chromium) {
                    w::sl();
                    rule->app_mode = w::icon_checkbox(ICON_MD_TAB_UNSELECTED, rule->app_mode);
                    w::tooltip("Open in chromeless window");
                }

                // scope (for "URL" rules)
                if(rule->loc == match_location::url) {
                    w::sl();
                    w::label("|", 0, false);
                    w::sl();
                    w::icon_list(url_scopes, (size_t&)rule->scope);
                }

                w::sl();
                if(w::button(ICON_MD_DELETE, w::emphasis::error)) {
                    //bi->rules.erase(bi->rules.begin() + i);
                }
                w::tooltip("Delete rule");
            }
        }
    }

    void config_app::add_custom_browser_by_asking() {
        string exe_path = win32::shell::file_open_dialog("Windows Executable", "*.exe");
        if(exe_path.empty()) return;

        wstring w_product_name = win32::user::get_file_version_info_string(exe_path, "ProductName");
        string id = win32::ole32::create_guid();
        string name = str::to_str(w_product_name);
        auto b = make_shared<browser>(id, name, exe_path, false);
        b->instances.push_back(make_shared<browser_instance>(b, "default", name, "", ""));

        vector<shared_ptr<browser>> all = browser::get_cache();
        all.push_back(b);
        g_config.save_browsers(all);
        browsers = browser::get_cache(true); // invalidate

        // find this new browser and select it (it won't be the last in the list)
        size_t idx = browser::index_of(browsers, b);
        if(idx != string::npos) {
            selected_browser_idx = idx;
        }
    }
}