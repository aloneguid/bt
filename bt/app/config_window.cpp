#include "config_window.h"
#include "discovery.h"
#include <iostream>
#include "str.h"
#include "win32/user.h"
#include "win32/shell.h"
#include "win32/ole32.h"
#include "win32/process.h"
#include "win32/clipboard.h"
#include "win32/sysinfo.h"
#include "datetime.h"
#include "../globals.h"
#include <filesystem>
#include <fmt/core.h>
#include "../res.inl"
#include "config.h"
#include "rule_hit_log.h"
#include "ui.h"
#include "update_check.h"

using namespace std;
using namespace grey;

namespace bt
{
    config_window::config_window(grey::grey_context& ctx)
        : grey::window{ctx, string{APP_LONG_NAME} + " " + APP_VERSION, 1000, 500}, gctx{ctx} {
        scale = get_system_scale();

        // just in case
        ui::ensure_no_instances();

        init();
    }

    void config_window::init() {
        has_menu_space = true;

        // restore from config
        // todo: move more here
        log_rule_hits = config::i.get_log_rule_hits();
        show_hidden_browsers = config::i.get_show_hidden_browsers();

        // build UI

        build_menu();

        auto panel_no_browsers = make_child_window();
        panel_no_browsers->is_visible = &panel_no_browsers_visible;
        for(int i = 0; i < 5; i++) panel_no_browsers->make_label("");
        panel_no_browsers->make_label(string(100, ' ')); panel_no_browsers->same_line();
        panel_no_browsers->make_label("Currently there are no browsers registered.")->set_emphasis(emphasis::primary);
        panel_no_browsers->make_label(string(80, ' ')); panel_no_browsers->same_line();
        panel_no_browsers->make_label("Press the button below to scan your system for installed browsers.");
        panel_no_browsers->make_label("");
        panel_no_browsers->make_label(string(110, ' ')); panel_no_browsers->same_line();
        auto cmd_discover = panel_no_browsers->make_button(
            "discover system browsers", false, emphasis::error);
        cmd_discover->on_pressed = [this](button&) {
            rediscover_browsers();
        };
        auto panel_left = make_child_window(400 * scale);
        panel_left->is_visible = &panel_left_visible;

        // before browser list starts
        auto cmd_add_custom = panel_left->make_button(ICON_FA_CIRCLE_PLUS " Add");
        cmd_add_custom->set_emphasis(emphasis::primary);
        cmd_add_custom->tooltip = "Add custom browser definition";
        cmd_add_custom->on_pressed = [this](button&) { add_custom_browser_by_asking(); };

        panel_left->same_line();
        auto chk_show_hidden = panel_left->make_checkbox(ICON_FA_EYE, &show_hidden_browsers);
        chk_show_hidden->render_as_icon = true;
        chk_show_hidden->tooltip = "show hidden browsers";
        chk_show_hidden->on_value_changed = [this](bool) { build_browsers(); };

        //w_browsers->has_border = true;
        panel_left->padding_bottom = 75 * scale;
        rpt_browsers = panel_left->make_repeater<browser>(
            [this](repeater_bind_context<browser> ctx) {
            render(ctx.data, ctx.container);
        }, true);
        rpt_browsers->on_item_clicked = [this](shared_ptr<container> c, shared_ptr<browser> b) {
            handle_selection(b);
        };

        same_line();
        auto panel_right = make_child_window();
        panel_right->is_visible = &panel_right_visible;
        panel_right->has_border = true;
        panel_right->padding_bottom = 80 * scale;
        browser_toolbar = panel_right->make_child_window(0, 38 * scale);
        //browser_toolbar->has_border = true;
        profiles_tabs = panel_right->make_tabs(true);
        profiles_tabs->is_visible = &profiles_tabs_visible;
        browser_free_area = panel_right->make_child_window();
        browser_free_area->is_visible = &browser_free_area_visible;

        build_browsers();

        menu->clicked = [this](menu_item& mi) {
            handle_menu_click(mi);
        };

        build_status_bar();

#if _DEBUG
        auto w_demo = make_demo();
        w_demo->is_visible = &demo_visible;
#endif

        w_dash = make_shared<dash_window>(gctx);
        w_dash->is_visible = &dash_visible;
        assign_child(w_dash);
        dash_visible = false;
        w_dash->on_health_changed = [this](bool healthy) {
            update_health_icon(healthy);
        };
        update_health_icon(w_dash->recheck());
    }

    void config_window::build_status_bar() {
        auto status = make_status_bar();
        st_health = status->make_label(ICON_FA_HEART);
        st_health->on_click = [this](component&) {
            bool healthy = w_dash->recheck();
            if(!healthy && !dash_visible) dash_visible = true;
        };

        status->make_label("|")->is_enabled = false;
        auto bc = status->make_label("");
        bc->tooltip = "Browser count";

        auto pc = status->make_label("");
        pc->tooltip = "Profile count";

        auto rc = status->make_label("");
        rc->tooltip = "Configured rule count";

        bc->is_enabled = pc->is_enabled = rc->is_enabled = false;

        status->on_frame = [this, bc, pc, rc](component&) {
            bc->set_value(fmt::format("{} {}", ICON_FA_WINDOW_RESTORE, browsers.size()));

            size_t ipc{0};
            size_t irc{0};
            for(const auto& b : browsers) {
                ipc += b->instances.size();
                for(const auto& i : b->instances) {
                    irc += i->rules.size();
                }
            }

            pc->set_value(fmt::format("{} {}", ICON_FA_USER, ipc));
            rc->set_value(fmt::format("{} {}", ICON_FA_RULER, irc));
        };
    }

    void config_window::refresh_proto_status(std::shared_ptr<grey::label> lbl, bool is) {
        if(is) {
            lbl->set_emphasis(emphasis::primary);
            lbl->tooltip = fmt::format("{} protocol is handled by {}", lbl->get_value(), APP_LONG_NAME);
        } else {
            lbl->set_emphasis(emphasis::error);
            lbl->tooltip = fmt::format("{} protocol is not handled by {},\ninterception will not work!", lbl->get_value(), APP_LONG_NAME);
        }
    }

    void config_window::build_menu() {
        menu = make_menu_bar();

        // FILE
        auto mi_file = menu->items()->add("", "File");
        mi_file->add("+b", "Add Custom Browser", ICON_FA_CIRCLE_PLUS);
        auto mi_ini = mi_file->add("", "config.ini", ICON_FA_GEARS);
        mi_ini->add("ini", "Open");
        mi_ini->add("ini+c", "Copy path to clipboard");
        auto mi_csv = mi_file->add("", "hit_log.csv", ICON_FA_BOOK);
        mi_csv->add("csv", "Open");
        mi_csv->add("csv+c", "Copy path to clipboard");
        mi_file->add("", "-");
        mi_file->add("x", "Exit", ICON_FA_ARROW_RIGHT_FROM_BRACKET);
        

        // TOOLS
        auto mi_tools = menu->items()->add("", "Tools");
        mi_tools->add("dash", "Readiness Dashboard", ICON_FA_GAUGE);
        mi_tools->add("test", "URL Tester", ICON_FA_TIMELINE);
        mi_tools->add("windows_defaults", "Windows Defaults", ICON_FA_WINDOWS);
        mi_tools->add("refresh", "Rediscover Browsers", ICON_FA_RETWEET);
        mi_tools->add("open_picker", "Test URL Picker", ICON_FA_CROSSHAIRS);
        auto mi_tbs = mi_tools->add("", "Troubleshooting");
        mi_tbs->add("fix_xbt", "Re-register Custom Protocol");
        mi_tbs->add("fix_browser", "Re-register as Browser");

        // SETTINGS
        auto mi_settings = menu->items()->add("", "Settings");
        mi_fallback = mi_settings->add("", "Default Browser");

        auto mi_open_mode = mi_settings->add("", "Browser Picker Mode");
        mi_om_silent = mi_open_mode->add("open_method_silent", "Never Ask");
        mi_om_decide = mi_open_mode->add("open_method_decide", "Ask on Conflict");
        mi_om_pick = mi_open_mode->add("open_method_pick", "Always Ask");
        set_open_method("");
        auto mi_phk = mi_open_mode->add("", "Also Open On");
        mi_phk_never = mi_phk->add("mi_phk_never", "Never");
        mi_phk_ctrlshift = mi_phk->add("mi_phk_cs", "CTRL+SHIFT+ " ICON_FA_ARROW_POINTER);
        mi_phk_ctrlalt = mi_phk->add("mi_phk_ca", "CTRL+ALT+ " ICON_FA_ARROW_POINTER);
        mi_phk_altshift = mi_phk->add("mi_phk_as", "SHIFT+ALT+ " ICON_FA_ARROW_POINTER);
        set_picker_hotkey("");

        auto mi_ff_mode = mi_settings->add("", "Firefox Container Mode", ICON_FA_FIREFOX);
        mi_ff_mode_off = mi_ff_mode->add("mi_ff_mode_off", "off (use profiles)", ICON_FA_POWER_OFF);
        mi_ff_mode_bt = mi_ff_mode->add("mi_ff_mode_bt", APP_LONG_NAME, ICON_FA_PUZZLE_PIECE);
        mi_ff_mode_ouic = mi_ff_mode->add("mi_ff_mode_ouic", "open-url-in-container", ICON_FA_PUZZLE_PIECE);
        update_firefox_mode(false, firefox_container_mode::off);

        auto mi_themes = mi_settings->add("", "Theme", ICON_FA_PAINT_ROLLER);
        for(auto& theme : gctx.list_themes()) {
            mi_themes->add(fmt::format("set_theme_{}", theme.id), theme.name);
        }

        auto mi_log_rule_hits = mi_settings->add("log_rule_hits", "Log Rule Hits to File");
        mi_log_rule_hits->is_selected = log_rule_hits;

        mi_settings->add("", "-");
        auto mi_cu = mi_settings->add("clearurls", "Enable ClearURLs", ICON_FA_TRASH_CAN);   // icon closely matches official
        mi_cu->is_selected = config::i.get_clearurls_enabled();

        // HELP
        auto mi_help = menu->items()->add("", "Help");

        auto mi_links = mi_help->add("", "Extensions", ICON_FA_PUZZLE_PIECE);
        mi_links->add("chrome_ex", "Google Chrome", ICON_FA_CHROME);
        mi_links->add("firefox_ex", "Mozilla Firefox", ICON_FA_FIREFOX);
        mi_links->add("edge_ex", "Microsoft Edge", ICON_FA_EDGE);

        mi_help->add("contact", "Contact", ICON_FA_ENVELOPE);
        mi_help->add("coffee", "Donate", ICON_FA_MUG_HOT);
        mi_help->add("releases", "All Releases", ICON_FA_CLOCK_ROTATE_LEFT);
        mi_help->add("check_version", "Check for Updates", ICON_FA_CODE_BRANCH);

        auto mi_reg = mi_help->add("", "Registry")->add("", "Copy path to clipboard");
        mi_reg->add("reg_xbt", "Custom Protocol");
        mi_reg->add("reg_browser", "Browser Registration");

        mi_help->add("", "-");
        mi_help->add("doc", "Documentation", ICON_FA_BOOK);
        mi_help->add("?", "About", ICON_FA_INFO);
#if _DEBUG
        mi_help->add("", "-");
        mi_help->add("demo", "Demo", ICON_FA_BOOK);
#endif
    }

    void config_window::handle_selection(shared_ptr<bt::browser> b) {

        // --- toolbar begin
        browser_toolbar->clear();

        // universal test button
        browser_toolbar->same_line();
        auto cmd_test = browser_toolbar->make_button(ICON_FA_SQUARE_UP_RIGHT);
        cmd_test->tooltip = "test by opening a link";
        cmd_test->on_click = [this, b](component&) {
            auto instance = b->instances[b->is_system ? profiles_tabs->get_selected_idx() : 0];
            url_payload pl{APP_URL};
            instance->launch(pl);
        };

        if(b->is_chromium) {
            browser_toolbar->same_line();
            auto cmd_test_app = browser_toolbar->make_button(ICON_FA_SQUARE_ARROW_UP_RIGHT);
            cmd_test_app->tooltip = "test by opening a link as an app";
            cmd_test_app->on_click = [this, b](component&) {
                auto instance = b->instances[b->is_system ? profiles_tabs->get_selected_idx() : 0];
                url_payload pl{APP_URL};
                pl.app_mode = true;
                instance->launch(pl);
            };
        }

        if(b->is_system) {

            browser_toolbar->same_line();
            auto g_static = browser_toolbar->make_group();
            if(!b->open_cmd.empty()) {
                auto bf = g_static->make_button(ICON_FA_FOLDER_OPEN);
                bf->tooltip = fmt::format("open {}'s folder in Explorer.", b->name);
                bf->on_pressed = [b](button&) {
                    std::filesystem::path p{b->open_cmd};
                    string path = p.parent_path().string();
                    win32::shell::exec(path, "");
                };

                if(b->is_firefox) {

                    auto cm = config::i.get_firefox_container_mode();

                    if(cm == firefox_container_mode::off) {
                        g_static->same_line();
                        auto pmgr = g_static->make_button(ICON_FA_ADDRESS_CARD);
                        pmgr->tooltip = "open Firefox Profile Manager (-P flag)";
                        pmgr->on_pressed = [b](button&) {
                            win32::shell::exec(b->open_cmd, "-P");
                        };

                        g_static->same_line();
                        pmgr = g_static->make_button(ICON_FA_ID_CARD);
                        pmgr->tooltip = "open Firefox Profile Manager in Firefox itself";
                        pmgr->on_pressed = [b](button&) {
                            win32::shell::exec(b->open_cmd, "about:profiles");
                        };
                    } else {

                        g_static->same_line();
                        auto cmd_x = g_static->make_button(ICON_FA_PUZZLE_PIECE);
                        cmd_x->tooltip = "download required extension";
                        cmd_x->set_emphasis(emphasis::warning);
                        cmd_x->on_pressed = [this, b](button&) {
                            // extenstion page needs to be opened in Firefox bypassing container mode
                            auto mode = config::i.get_firefox_container_mode();
                            string url = mode == firefox_container_mode::bt
                                ? APP_BROWSER_EXTENSION_FIREFOX_URL
                                : APP_BROWSER_EXTENSION_FIREFOX_OUIC_URL;
                            win32::shell::exec(b->open_cmd, url);
                        };
                    }
                }
                else if(b->is_chromium) {
                    g_static->same_line();
                    auto cmd_x = g_static->make_button(ICON_FA_PUZZLE_PIECE);
                    cmd_x->tooltip = "download optional integration extension";
                    cmd_x->on_pressed = [this, b](button&) {
                        // extenstion page needs to be opened in the correct profile
                        auto instance = b->instances[b->is_system ? profiles_tabs->get_selected_idx() : 0];
                        string url = b->id == "msedge"
                            ? APP_BROWSER_EXTENSION_EDGE_URL
                            : APP_BROWSER_EXTENSION_CHROME_URL;
                        instance->launch(url_payload{url});
                    };

                }

            }
        } else {
            browser_toolbar->same_line();
            auto cmd_rm = browser_toolbar->make_button(
                ICON_FA_TRASH " delete", false, grey::emphasis::error);
            cmd_rm->tooltip = "Completely deletes this browser, no questions asked";
            cmd_rm->on_pressed = [this, b](grey::button&) {

                size_t idx = index_of(b);

                // erase and save
                vector<shared_ptr<browser>> all = browser::get_cache();
                std::erase_if(all, [b](auto i) { return i->id == b->id; });
                config::i.save_browsers(all);
                browser::get_cache(true); // invalidate

                // rebuild profiles
                build_browsers();

                // if possible, select previous browser
                if(idx != string::npos) {
                    idx -= 1;
                    if(idx >= 0 && idx < browsers.size()) {
                        rpt_browsers->set_selected_index(idx);
                        handle_selection(browsers[idx]);
                    }
                }
            };
        }

        // hide/show button
        browser_toolbar->same_line();
        auto chk_show = browser_toolbar->make_checkbox(ICON_FA_EYE);
        chk_show->tooltip = "Show or hide this browser from the browser list";
        chk_show->render_as_icon = true;
        chk_show->set_checked(!b->is_hidden);
        chk_show->on_value_changed = [this, b](bool is_visible) {
            b->is_hidden = !is_visible;
            persist_ui();
        };


        // --- toolbar end

        // profiles
        profiles_tabs->clear();
        browser_free_area->clear();

        if(b->is_system) {
            profiles_tabs_visible = true;
            browser_free_area_visible = false;

            for(auto bi : b->instances) {
                string tab_icon;
                if(bi->is_incognito) {
                    tab_icon = fmt::format("{} ", ICON_FA_GLASSES);
                }
                auto tab = profiles_tabs->make(fmt::format(" {}{} ", tab_icon, bi->name));

                tab->spacer();

                auto acc_params = tab->make_accordion("Parameters");
                {
                    // basic metadata

                    //auto txt_exe_path = acc_params->make_input("exe");
                    //txt_exe_path->set_value(b->open_cmd);
                    //txt_exe_path->set_select_all_on_focus();
                    //txt_exe_path->set_is_readonly();
                    //txt_exe_path->tooltip = "full path to browser executable.";

                    auto txt_arg = acc_params->make_input("arg");
                    txt_arg->tooltip = "Discovered arguments (read-only)";
                    txt_arg->set_value(bi->launch_arg);
                    txt_arg->set_select_all_on_focus();
                    txt_arg->set_is_readonly();

                    auto txt_user_arg = acc_params->make_input("extra arg");
                    txt_user_arg->tooltip = "Any extra arguments to pass.\nIf you break it, you fix it ;)";
                    txt_user_arg->set_value(bi->user_arg);
                    txt_user_arg->on_value_changed = [this, bi](string& v) {
                        string uv = v;
                        str::trim(uv);
                        bi->user_arg = uv;
                        persist_ui();
                    };
                }

                // rules
                tab->spacer();
                bind_edit_rules(tab, bi);
            }
        } else {
            profiles_tabs_visible = false;
            browser_free_area_visible = true;
            auto bi = b->instances[0];

            browser_free_area->make_label("Parameters");

            auto txt_exe_path = browser_free_area->make_input("exe");
            txt_exe_path->set_value(b->open_cmd);
            txt_exe_path->set_select_all_on_focus();
            txt_exe_path->set_is_readonly();
            txt_exe_path->tooltip = "Full path to browser executable. The only way to change this is to re-create the browser. Sorry ;)";

            auto txt_name = browser_free_area->make_input("name");
            txt_name->set_value(bi->name);
            txt_name->on_value_changed = [this, b, bi](string& new_name) {
                bi->name = new_name;
                b->name = new_name;
                persist_ui();
            };

            auto txt_arg = browser_free_area->make_input("arg");
            txt_arg->set_value(bi->launch_arg);
            txt_arg->on_value_changed = [this, bi](string& new_arg) {
                bi->launch_arg = new_arg;
                persist_ui();
            };
            txt_arg->tooltip =
                R"(Argument(s) to pass to the browser.
It is empty by default and opening url is always passed as an argument.
If you set this value, it is used as is. Also, 'arg' can contain a
special keyword - %url% which is replaced by opening url.)";


            // rules
            browser_free_area->spacer();
            bind_edit_rules(browser_free_area, bi);
        }
    }

    void config_window::handle_menu_click(grey::menu_item& mi) {
        if(mi.id == "+b") {
            add_custom_browser_by_asking();
        } else if(mi.id == "x") {
            ::PostQuitMessage(0);
        } else if(mi.id == "ini") {
            win32::shell::exec(config::i.get_absolute_path(), "");
        } else if(mi.id == "ini+c") {
            win32::clipboard::set_ascii_text(config::i.get_absolute_path());
        } else if(mi.id == "csv") {
            win32::shell::exec(rule_hit_log::i.get_absolute_path(), "");
        } else if(mi.id == "csv+c") {
            win32::clipboard::set_ascii_text(rule_hit_log::i.get_absolute_path());
        } else if(mi.id == "contact") {
            bt::ui::contact();
        } else if(mi.id == "coffee") {
            bt::ui::coffee("menu");
        } else if(mi.id == "check_version") {
            string vn;
            if(bt::app::has_new_version(vn)) {
                app_event("new_version", vn, "");
            }
        } else if(mi.id == "releases") {
            ui::url_open(
               url_payload{APP_GITHUB_RELEASES_URL},
               ui::open_method::configured);
        } else if(mi.id == "doc") {
            ui::url_open(
                url_payload{APP_DOCS_URL},
                ui::open_method::configured);
        } else if(mi.id == "?") {
            auto w = gctx.make_window<about_window>();
            w->detach_on_close = true;
            w->center();
        } else if(mi.id == "demo") {
#if _DEBUG
            demo_visible = true;
#endif
        } else if(mi.id == "reg_xbt") {
            win32::clipboard::set_ascii_text(
                fmt::format("Computer\\HKEY_CURRENT_USER\\{}", bt::setup::get_custom_proto_reg_path()));
        } else if(mi.id == "fix_xbt") {
            bt::setup::register_protocol();
        } else if(mi.id == "reg_browser") {
            win32::clipboard::set_ascii_text(
                fmt::format("Computer\\HKEY_CURRENT_USER\\{}", bt::setup::get_browser_registration_reg_path()));
        } else if(mi.id == "fix_browser") {
            bt::setup::register_browser();
        } else if(mi.id == "dash") {
            dash_visible = true;
        } else if(mi.id == "test") {
            ui::url_tester();
        } else if(mi.id == "windows_defaults") {
            win32::shell::open_default_apps();
        } else if(mi.id == "refresh") {
            rediscover_browsers();
        } else if(mi.id == "open_picker") {
            ui::url_open(bt::url_payload{"https://aloneguid.uk"}, ui::open_method::pick);
        } else if(mi.id == "update") {
            // todo: open direct url
            ui::url_open(
               url_payload{string(APP_URL) + "#installing"},
               ui::open_method::configured);
        } else if(mi.id == "chrome_ex") {
            ui::url_open(
               url_payload{APP_BROWSER_EXTENSION_CHROME_URL},
               ui::open_method::configured);
        } else if(mi.id == "firefox_ex") {
            ui::url_open(
               url_payload{APP_BROWSER_EXTENSION_FIREFOX_URL},
               ui::open_method::configured);
        } else if(mi.id == "edge_ex") {
            ui::url_open(
               url_payload{APP_BROWSER_EXTENSION_EDGE_URL},
               ui::open_method::configured);
        } else if(mi.id == "picker") {
            config::i.set_picker_enabled(!mi.is_selected);
            mi.is_selected = !mi.is_selected;
            ui::ensure_no_instances();
        } else if(mi.id == "log_rule_hits") {
            mi.is_selected = !mi.is_selected;
            config::i.set_log_rule_hits(mi.is_selected);
            ui::ensure_no_instances();
        } else if(mi.id.starts_with("set_fallback_")) {
            string lsn = mi.id.substr(13);
            bool found{false};
            auto bi = browser::find_profile_by_long_id(browsers, lsn, found);
            if(found) {
                set_fallback(bi);

                build_default_browser_menu();
            }
        } else if(mi.id.starts_with("open_method_")) {
            set_open_method(mi.id.substr(12));
        } else if(mi.id.starts_with("mi_phk_")) {
            set_picker_hotkey(mi.id.substr(7));
        } else if(mi.id.starts_with("set_theme_")) {
            string id = mi.id.substr(10);
            gctx.set_theme(id);
            config::i.set_theme(id);
        } else if(mi.id.starts_with("mi_ff_mode_")) {
            string name = mi.id.substr(11);
            update_firefox_mode(true, config::to_firefox_container_mode(name));
        } else if(mi.id == "clearurls") {
            mi.is_selected = !mi.is_selected;
            config::i.set_clearurls_enabled(mi.is_selected);
            pipeline.reconfigure();
        }
    }

    void config_window::render(shared_ptr<bt::browser> b, shared_ptr<container> c) {

        float padding = 10 * scale;
        float icon_size = 32 * scale;
        float left_pad = icon_size + padding * 2;

        // render icon and come back to starting position
        c->set_pos(0, -1, false);
        c->set_pos(padding, padding, true);
        if(b->open_cmd.empty() || !std::filesystem::exists(b->open_cmd)) {
            c->make_image_from_memory("logo", icon_png, icon_png_len,
                32 * scale, 32 * scale);
        } else {
            c->make_image_from_file(b->open_cmd, 32 * scale, 32 * scale);
        }
        c->set_pos(0, -1);
        c->set_pos(0, -(icon_size + padding), true);


        // elements
        c->set_pos(0, -1, false);
        c->set_pos(left_pad, 0, true);
        auto txt_name = c->make_label(b->name);

        c->set_pos(left_pad, 0, true);

        if(b->instances.size() > 0) {
            if(b->is_system) {
                auto i_where = c->make_label(fmt::format("{} {}", ICON_FA_USER, b->instances.size()));
                i_where->tooltip = str::humanise(b->instances.size(), "profile", "profiles");
                i_where->is_enabled = false;

                if(b->is_chromium) {
                    c->same_line();
                    auto sign_cb = c->make_label(ICON_FA_CHROME);
                    sign_cb->is_enabled = false;
                    sign_cb->tooltip = "Chromium-based";
                } else if(b->is_firefox) {
                    c->same_line();
                    auto sign_fb = c->make_label(ICON_FA_FIREFOX);
                    sign_fb->is_enabled = false;
                    sign_fb->tooltip = "Firefox-based";
                }
            } else {
                auto lbl_custom = c->make_label(ICON_FA_PERSON_MILITARY_POINTING);
                lbl_custom->is_enabled = false;
                lbl_custom->tooltip = "User-defined";
            }

            if(b->get_supports_frameless_windows()) {
                auto i = c->make_label(ICON_FA_CROP,
                    "Supports frameless windows", false, true);
            }

            auto chk_hidden = c->make_label(ICON_FA_EYE_SLASH);
            chk_hidden->same_line = true;
            chk_hidden->is_enabled = false;
            chk_hidden->tooltip = "Hidden";
            chk_hidden->is_visible = &(b->is_hidden);

        } else {
            auto i_where = c->make_label(ICON_FA_USER);
            i_where->tooltip = "no profiles";
            i_where->is_enabled = false;
        }

        c->spacer();
        c->set_pos(0, -1);

        // a hack for dynamic updates
        c->on_frame = [b, txt_name](component&) {
            if(!b->is_system) {
                txt_name->set_value(b->name);
            }
        };
    }

    void config_window::build_browsers() {
        browsers = browser::get_cache();

        if(browsers.empty()) {
            panel_no_browsers_visible = true;
            panel_left_visible = panel_right_visible = false;
        } else {
            panel_no_browsers_visible = false;
            panel_left_visible = panel_right_visible = true;

            vector<shared_ptr<browser>> filtered;
            if(show_hidden_browsers) {
                filtered = browsers;
            } else {
                std::copy_if(browsers.begin(), browsers.end(),
                    back_inserter(filtered),
                    [](shared_ptr<browser> i) { return !i->is_hidden; });
            }

            rpt_browsers->clear();

            if(!filtered.empty()) {
                rpt_browsers->bind(filtered);

                build_default_browser_menu();
                handle_selection(*filtered.begin());
                rpt_browsers->set_selected_index(0);
            }
        }
    }

    void config_window::rediscover_browsers() {
        vector<shared_ptr<browser>> fresh_browsers = discovery::discover_all_browsers();
        fresh_browsers = browser::merge(fresh_browsers, browser::get_cache());
        config::i.save_browsers(fresh_browsers);

        browser::get_cache(true);   // invalidate cache

        build_browsers();
    }

    void config_window::add_custom_browser_by_asking() {
        string exe_path = win32::shell::file_open_dialog("Windows Executable", "*.exe");
        if(exe_path.empty()) return;

        wstring w_product_name = win32::user::get_file_version_info_string(exe_path, "ProductName");
        string id = win32::ole32::create_guid();
        string name = str::to_str(w_product_name);
        auto b = make_shared<browser>(id, name, exe_path, false);
        b->instances.push_back(make_shared<browser_instance>(b, "default", name, "", ""));

        vector<shared_ptr<browser>> all = browser::get_cache();
        all.push_back(b);
        config::i.save_browsers(all);
        browser::get_cache(true); // invalidate

        build_browsers();

        // find this new browser and select it
        size_t idx = index_of(b);
        if(idx != string::npos) {
            rpt_browsers->set_selected_index(idx);
            handle_selection(browsers[idx]);
        }
    }

    void config_window::build_default_browser_menu() {
        // menu item
        mi_fallback->children.clear();
        fb = browser::get_fallback(browsers);
        for(auto b : browsers) {
            auto mi_b = mi_fallback->add("", b->name);
            for(auto bi : b->instances) {
                auto mi_bi = mi_b->add(string{"set_fallback_"} + bi->long_id(), bi->name);
                if(fb->long_id() == bi->long_id()) mi_bi->is_selected = true;
            }
        }
    }

    void config_window::bind_edit_rules(std::shared_ptr<grey::container> tab, shared_ptr<browser_instance> bi) {

        tab->make_label("Rules");
        auto radd = tab->make_button(ICON_FA_CIRCLE_PLUS " add");
        radd->set_emphasis(emphasis::primary);

        tab->same_line();
        auto rca = tab->make_button(ICON_FA_TRASH " clear all");
        rca->set_emphasis(emphasis::warning);

        tab->same_line();
        auto rt = tab->make_button(ICON_FA_TIMELINE " test");
        rt->tooltip = "Open URL Tester";

        auto rw = tab->make_child_window();
        rw->has_border = true;  // for debug only

        auto rpt_rules = rw->make_repeater<bt::match_rule>(
            [this, bi](repeater_bind_context<bt::match_rule> ctx) {

                shared_ptr<grey::container> ctr = ctx.container;
                shared_ptr<match_rule> rule = ctx.data;

                // value
                auto txt_value = ctr->make_input("");
                txt_value->set_value(rule->value);
                txt_value->width = 250 * scale;

                // is regex checkbox
                ctr->same_line();
                auto chk_regex = ctr->make_checkbox(ICON_FA_TRUCK_FAST);
                chk_regex->render_as_icon = true;
                chk_regex->tooltip = "Rule is a Regular Expression (advanced)";
                chk_regex->set_checked(rule->is_regex);

                // priority
                /*ctr->same_line();
                auto chk_priority = ctr->make_checkbox(ICON_FA_ARROW_UP_9_1);
                chk_priority->render_as_icon = true;
                chk_priority->tooltip = "If multiple rules will match an URL, you can override default priority";
                chk_priority->set_checked(ctx.data->priority != 0);

                ctr->same_line();
                auto txt_priority = ctr->make_input_int("");
                txt_priority->set_value(ctx.data->priority);
                txt_priority->width = 20 * scale;
                txt_priority->set_step_button_step(0);
                txt_priority->tooltip = "If multiple rules will match an URL, this value indicates priority.";
                txt_priority->is_visible = ctx.data->priority != 0;*/

                // app mode
                if(bi->b->is_chromium) {
                    ctr->same_line();
                    auto chk_app_mode = ctr->make_checkbox(ICON_FA_CROP_SIMPLE);
                    chk_app_mode->render_as_icon = true;
                    chk_app_mode->set_checked(ctx.data->app_mode);
                    chk_app_mode->tooltip = "open in chromeless window";

                    chk_app_mode->on_value_changed = [this, ctx](bool set) {
                        ctx.data->app_mode = set;
                        persist_ui();
                    };
                }

                // firefox container
                if(bi->b->is_firefox) {
                    ctr->same_line();
                    auto chk_ct = ctr->make_checkbox(ICON_FA_BOX);
                    chk_ct->render_as_icon = true;
                    chk_ct->tooltip = "open in Firefox container";
                }

                // scope
                ctr->same_line();
                ctr->make_label("|")->is_enabled = false;
                ctr->same_line();
                auto lst_scope = ctr->make_listbox("");
                lst_scope->mode = listbox_mode::icons;
                lst_scope->items.push_back(list_item{ICON_FA_GLOBE, "match anywhere"});
                lst_scope->items.push_back(list_item{ICON_FA_LANDMARK_DOME, "match only in host name"});
                lst_scope->items.push_back(list_item{ICON_FA_LINES_LEANING, "match only in path"});
                lst_scope->selected_index = static_cast<size_t>(ctx.data->scope);

                ctr->same_line();
                auto rm = ctr->make_button(ICON_FA_DELETE_LEFT);
                rm->set_emphasis(emphasis::error);
                rm->tooltip = "delete rule";

                //ctr->make_label("");
                //ctr->same_line(25 * scale);
                //ctr->make_input(ICON_FA_BOX);

                // handlers

                lst_scope->on_selected = [this, rule](size_t idx, grey::list_item&) {
                    rule->scope = (match_scope)idx;
                    persist_ui();
                };

                chk_regex->on_value_changed = [this, rule](bool is_regex) {
                    rule->is_regex = is_regex;
                    persist_ui();
                };

                txt_value->on_value_changed = [this, rule](string& s) {
                    rule->value = s;
                    persist_ui();
                };

                auto move_rule = [this, bi, rule, ctx](int pos) {
                    // find rule index
                    int idx = -1;
                    for(int i = 0; i < bi->rules.size(); i++) {
                        if(bi->rules[i] == rule) {
                            idx = i;
                            break;
                        }
                    }
                    if(idx == -1) return;

                    ctx.rpt.move(idx, pos, true);
                    stl::move(bi->rules, idx, pos, true);
                    persist_ui();
                };

                txt_value->on_arrow_up = [this, bi, rule, ctx, move_rule]() {
                    move_rule(-1);
                };

                txt_value->on_arrow_down = [this, bi, rule, ctx, move_rule]() {
                    move_rule(1);
                };


                /*chk_priority->on_value_changed = [this, txt_priority, ctx](bool set) {
                    //txt_priority->is_visible = set;
                    if(set) {
                        //
                    } else {
                        txt_priority->set_value(0);
                        ctx.data->priority = 0;
                    }
                    persist_ui();
                };
                txt_priority->on_value_changed = [this, bi, ctx](int& v) {
                    ctx.data->priority = v;
                    persist_ui();
                };*/

                rm->on_pressed = [this, bi, ctx](button&) {
                    bi->delete_rule(ctx.data->value);
                    persist_ui();
                    ctx.rpt.bind(bi->rules);
                };
        });
        rpt_rules->bind(bi->rules);

        // key handlers

        radd->on_pressed = [this, bi, rpt_rules](button&) {
            bi->add_rule(fmt::format("rule {}", bi->rules.size()));
            browser::persist_cache();
            ui::ensure_no_instances();
            rpt_rules->bind(bi->rules);
        };

        rca->on_pressed = [this, bi, rpt_rules](button&) {
            bi->rules.clear();
            browser::persist_cache();
            ui::ensure_no_instances();
            rpt_rules->bind(bi->rules);
        };

        rt->on_pressed = [this](button&) {
            ui::url_tester();
        };
    }

    void config_window::set_fallback(std::shared_ptr<browser_instance> bi) {
        config::i.set_fallback(bi->long_id());
        build_browsers();
        ui::ensure_no_instances();
    }

    void config_window::set_open_method(const std::string& update_to) {
        mi_om_silent->is_selected =
            mi_om_decide->is_selected =
            mi_om_pick->is_selected = false;

        string to = update_to;

        if(to.empty()) {
            to = config::i.get_open_method();
        } else {
            config::i.set_open_method(to);
        }

        if(to == "silent") {
            mi_om_silent->is_selected = true;
        } else if(to == "decide") {
            mi_om_decide->is_selected = true;
        } else if(to == "pick") {
            mi_om_pick->is_selected = true;
        }
    }

    void config_window::set_picker_hotkey(const std::string& update_to) {
        mi_phk_never->is_selected =
            mi_phk_ctrlalt->is_selected =
            mi_phk_ctrlshift->is_selected =
            mi_phk_altshift->is_selected = false;

        string to = update_to;

        // update config if required
        if(to.empty()) {
            if(config::i.get_picker_enabled()) {
                to = config::i.get_picker_hotkey();
            } else {
                to = "never";
            }
        } else {
            if(to == "never") {
                config::i.set_picker_enabled(false);
            } else {
                config::i.set_picker_enabled(true);
                config::i.set_picker_hotkey(to);
            }
        }

        if(to == "never") {
            mi_phk_never->is_selected = true;
        } else if(to == "cs") {
            mi_phk_ctrlshift->is_selected = true;
        } else if(to == "ca") {
            mi_phk_ctrlalt->is_selected = true;
        } else if(to == "as") {
            mi_phk_altshift->is_selected = true;
        }
    }

    void config_window::update_firefox_mode(bool update, firefox_container_mode mode) {
        mi_ff_mode_off->is_selected = mi_ff_mode_bt->is_selected = mi_ff_mode_ouic->is_selected = false;

        if(update) {
            config::i.set_firefox_container_mode(mode);
        }

        mode = config::i.get_firefox_container_mode();
        switch(mode) {
            case bt::firefox_container_mode::off:
                mi_ff_mode_off->is_selected = true;
                break;
            case bt::firefox_container_mode::bt:
                mi_ff_mode_bt->is_selected = true;
                break;
            case bt::firefox_container_mode::ouic:
                mi_ff_mode_ouic->is_selected = true;
                break;
            default:
                break;
        }
    }

    void config_window::update_health_icon(bool healthy) {
        if(!healthy) {
            dash_visible = true;
            w_dash->center();
            w_dash->bring_to_top();
        }
        st_health->set_emphasis(healthy ? emphasis::primary : emphasis::error);
        st_health->tooltip = healthy
            ? "good health"
            : "health issues detected";
    }

    void config_window::persist_ui() {
        browser::persist_cache();
        ui::ensure_no_instances();
    }

    size_t config_window::index_of(std::shared_ptr<bt::browser> b) {
        string id = b->id;
        auto bit = std::find_if(browsers.begin(), browsers.end(),
            [id](const shared_ptr<browser>& i) {return i->id == id; });

        if(bit != browsers.end()) {
            size_t idx = bit - browsers.begin();
            return idx;
        }

        return string::npos;
    }
}