#include "config_app.h"
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
#include "../app_log.h"
#include <filesystem>
#include "../url_opener.h"
#include "../discovery.h"
#include "../pipeline/replacer.h"
#include "../../globals.h"
#include "../strings.h"
#include "extra_widgets.hpp"
#include "datetime.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    config_app::config_app() :
        title{string{APP_LONG_NAME} + " " + APP_VERSION},
        wnd_config{title, &is_open},
        wnd_about{"About"},
        wnd_subs{"Substitutions", &show_subs},
        wnd_scripting{strings::ScriptEditor, &show_scripting},
        wnd_pv{strings::PipelineDebugger, &pv_show} {

        app = grey::app::make(title, 900, 500);
        app->initial_theme_id = g_config.theme_id;
        app->win32_can_resize = false;
        app->win32_center_on_screen = true;

        wnd_config
            .has_menubar()
            .no_titlebar()
            .border(0)
            .no_resize()
            .no_collapse()
            .fill_viewport()
            .no_scroll();

        wnd_about
            .size(310, 355)
            .center()
            .border(1)
            .no_resize();

        wnd_subs
            .size(600, 300)
            .border(1)
            .center();

        wnd_scripting
            .size(600, 800)
            .border(1)
            .no_scroll();

        wnd_pv
            .size(800, 500)
            .border(1)
            .center();

        float padding_bottom = 20 * app->scale;
        w_left_panel = w::container{250 * app->scale, -padding_bottom};
        w_right_panel = w::container{0, -padding_bottom}.border();

        app->on_initialised = [this]() {
            app->preload_texture("logo", icon_png, icon_png_len);
            app->preload_texture("incognito", incognito_icon_png, incognito_icon_png_len);
            app->preload_texture("bt_chromium", chromium_icon_png, chromium_icon_png_len);
            app->preload_texture("bt_gecko", gecko_icon_png, gecko_icon_png_len);
        };

        // in case config is not set, explicitly set it to default
        if(!g_config.browsers.empty()) {
            g_config.default_profile_long_id =
                browser::get_default(g_config.browsers, g_config.default_profile_long_id)->long_id();
        }

        check_health();
    }

    config_app::~config_app() {
        g_config.commit();
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
        w::guard gw{wnd_config};

        render_menu_bar();

        if(g_config.browsers.empty()) {
            render_no_browsers();
        } else {
            render_browsers();
        }

#if _DEBUG
        if(show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }
#endif

        render_status_bar();

        if(show_about)
            render_about_window();

        if(show_subs)
            render_subs_window();

        if(show_scripting)
            render_scripting_window();

        if(pv_show)
            render_pipe_visualiser_window();

        render_dashboard();

        w::notify_render_frame();

        return is_open;
    }

    void config_app::render_menu_bar() {
        //w::menu_bar menu{menu_items, [this](const string& id) { handle_menu_click(id); }};

        if(w::menu_bar menu; menu) {

            if(w::menu m_file{"File"}; m_file) {
                if(w::mi("Save configuration", true, ICON_MD_SAVE)) {
                    g_config.commit();
                    w::notify_info("Configuration saved.");
                }
                if(w::mi("Add custom browser", true, ICON_MD_ADD_CIRCLE)) {
                    add_custom_browser_by_asking();
                }
               

                if(w::menu m_ini{"config.ini", true, ICON_MD_SETTINGS}; m_ini) {
                    if(w::mi("Open")) {
                        win32::shell::exec(g_config.get_absolute_path(), "");
                    }
                    if(w::mi("Copy path")) {
                        win32::clipboard::set_ascii_text(g_config.get_absolute_path());
                        w::notify_info("Path copied to clipboard.");
                    }
                }

                if(w::menu m_csv{"hit_log.csv", true, ICON_MD_BOOK}; m_csv) {
                    if(w::mi("Open")) {
                        win32::shell::exec(rule_hit_log::i.get_absolute_path(), "");
                    }
                    if(w::mi("Copy path")) {
                        win32::clipboard::set_ascii_text(rule_hit_log::i.get_absolute_path());
                        w::notify_info("Path copied to clipboard.");
                    }
                }

                if(w::menu m_app_log{"log.txt", true, ICON_MD_BOOK}; m_app_log) {
                    if(w::mi("Open")) {
                        win32::shell::exec(app_log::i.get_absolute_path(), "");
                    }
                    if(w::mi("Copy path")) {
                        win32::clipboard::set_ascii_text(app_log::i.get_absolute_path());
                        w::notify_info("Path copied to clipboard.");
                    }
                }

                w::sep();
                if(w::mi("Exit", true, ICON_MD_LOGOUT)) {
                    is_open = false;
                }
            }

            if(w::menu m_tools{strings::MenuTools}; m_tools) {
                if(w::mi("Open Windows Defaults", true, ICON_MD_PSYCHOLOGY)) {
                    win32::shell::open_default_apps();
                }
                if(w::mi("Rediscover Browsers", true, ICON_MD_REFRESH)) {
                    rediscover_browsers();
                }
                if(w::mi(strings::PipelineDebugger, true, ICON_MD_DIRECTIONS_RUN)) {
                    pv_show = !pv_show;
                }
                if(w::mi(strings::ScriptEditor, true, ICON_MD_CODE)) {
                    show_scripting = !show_scripting;
                }
                if(w::menu m{"Troubleshooting", true}; m) {
                    if(w::mi("Re-register custom protocol")) {
                        w::notify_info("todo");
                    }
                    if(w::mi("Copy custom protocol")) {
                        win32::clipboard::set_ascii_text(fmt::format("Computer\\HKEY_CURRENT_USER\\{}", bt::setup::get_custom_proto_reg_path()));
                        w::notify_info("Registry path copied to clipboard.");
                    }
                    if(w::mi("Copy browser registration")) {
                        win32::clipboard::set_ascii_text(fmt::format("Computer\\HKEY_CURRENT_USER\\{}", bt::setup::get_browser_registration_reg_path()));
                        w::notify_info("Registry path copied to clipboard.");
                    }
                    if(w::mi("Re-register as browser")) {
                        w::notify_info("todo");
                    }
                }
            }

            if(w::menu m{"General"}; m) {
                w::small_checkbox("Write clicks to hit_log.csv", g_config.log_rule_hits);
                w::small_checkbox("Log application events to log.txt", g_config.log_app);

                w::mi_themes([this](const string& theme_id) {
                    app->set_theme(theme_id);
                    g_config.theme_id = theme_id;
                });

                w::sep("Firefox container mode");
                if(w::small_radio("Off (use legacy profiles)", g_config.firefox_mode == firefox_container_mode::off)) {
                    g_config.firefox_mode = firefox_container_mode::off;
                }
                if(w::small_radio(APP_LONG_NAME, g_config.firefox_mode == firefox_container_mode::bt)) {
                    g_config.firefox_mode = firefox_container_mode::bt;
                }
                if(w::small_radio("open-url-in-container", g_config.firefox_mode == firefox_container_mode::ouic)) {
                    g_config.firefox_mode = firefox_container_mode::ouic;
                }
            }

            if(w::menu m{"Picker"}; m) {
                w::sep("Manual invocation");
                w::small_checkbox("Ctrl + Shift + Left Click", g_config.picker_on_key_cs);
                w::small_checkbox("Ctrl + Alt    + Left Click", g_config.picker_on_key_ca);
                w::small_checkbox("Alt  + Shift + Left Click", g_config.picker_on_key_as);

                w::sep("Automatic invocation");
                w::small_checkbox("Always", g_config.picker_always);
                if(!g_config.picker_always) {
                    w::small_checkbox("On conflict", g_config.picker_on_conflict);
                    w::small_checkbox("On no rule", g_config.picker_on_no_rule);
                }
            }

            if(w::menu m{"Pipeline"}; m) {
                if(w::small_checkbox("Unwrap Office 365 links", g_config.pipeline_unwrap_o365)) {
                    g_pipeline.load();
                }
                if(w::small_checkbox("Unshorten links", g_config.pipeline_unshorten)) {
                    g_pipeline.load();
                }
                if(w::mi("Substitutions...", true, ICON_MD_FIND_REPLACE)) {
                    show_subs = !show_subs;
                }
            }

            if(w::menu m{"Help"}; m) {
                if(w::mi("Extensions", true, ICON_MD_OPEN_IN_NEW)) {
                    url_opener::open(APP_BROWSER_EXTENSIONS_DOCS_URL);
                }
                if(w::mi("Contact", true, ICON_MD_OPEN_IN_NEW)) {
                    url_opener::open("https://www.aloneguid.uk/about/#contact");
                }
                if(w::mi("All Releases", true, ICON_MD_OPEN_IN_NEW)) {
                    url_opener::open(APP_GITHUB_RELEASES_URL);
                }
                if(w::mi("Documentation", true, ICON_MD_OPEN_IN_NEW)) {
                    url_opener::open(APP_DOCS_URL);
                }
                if(w::mi("About", true, ICON_MD_INFO)) {
                    show_about = !show_about;
                }
#if _DEBUG
                if(w::mi("ImGui demo", true)) {
                    show_demo = !show_demo;
                }
#endif
            }
        }
    }

    void config_app::startup_health_warning() {
        if(health_failed > 0) {

            string title = "Health warning";

            if (!startup_health_opened) {
                ImGui::OpenPopup(title.c_str());
                startup_health_opened = true;
            }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                string msg = fmt::format(
                    "There are {} issues preventing " APP_SHORT_NAME " from working properly.", health_failed);

                if (w::button("Close", w::emphasis::error)) {
                    ImGui::CloseCurrentPopup();
                    startup_health_warned = true;
                }

                ImGui::EndPopup();
            }
        }
        else {
            startup_health_warned = true;
        }
    }

    void config_app::render_about_window() {
        const int width = 310;
        app->preload_texture("logo", icon_png, icon_png_len);
        w::guard gw{wnd_about};

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
            url_opener::open(APP_URL);
        }
        w::sl();
        if(w::button("GitHub")) {
            url_opener::open(APP_GITHUB_URL);
        };
        w::sl();
        if(w::button("Close")) {
            show_about = false;
        }
    }

    void config_app::render_subs_window() {
        w::guard gw{wnd_subs};

        bool recompute{false};

        if(w::button(ICON_MD_ADD " add", w::emphasis::primary)) {
            g_config.pipeline_substitutions.push_back("");
            g_pipeline.load();
            recompute = true;
        }

        w::sl(); ww::help_link("url-proc.html#substitutions");

        // testing stuff
        w::sep("Test");
        if(w::input(url_subs_up.url, ICON_MD_LOGIN)) recompute = true;
        w::tooltip("Input a value to test substitutions.");
        if(w::input(url_subs_up.url, ICON_MD_LOGOUT, true, 0, true)) recompute = true;
        w::tooltip("Substitution result are populated here.");

        w::sep("Substitutions");
        // scrollable container of subs
        w::container scr{"scr"};
        scr.border();
        {
            w::guard g{scr};
            
            for(int i = 0; i < g_config.pipeline_substitutions.size(); i++) {
                if(i > 0) w::sep();
                string suffix = "##" + to_string(i);

                auto replacer = g_pipeline.get_replacer_step(i);

                float pad = 100.0f * app->scale + 10 * app->scale;
                float iw = 250.0f * app->scale;

                w::combo("##kind" + suffix,
                    replacer_kinds,
                    (size_t&)replacer->kind,
                    100);

                w::sl(pad);
                if(w::input(replacer->find, "find" + suffix, true, iw)) recompute = true;

                // "delete" button to the right
                w::sl(pad + iw + 70 * app->scale);
                if(w::button(ICON_MD_DELETE + suffix, w::emphasis::error)) {
                    g_config.pipeline_substitutions.erase(g_config.pipeline_substitutions.begin() + i);
                    g_pipeline.load();
                    recompute = true;
                    break;
                }

                w::label(""); w::sl(pad);
                if(w::input(replacer->replace, "replace" + suffix, true, iw)) recompute = true;
            }
        }

        if(recompute) {

            // sync state back to config
            g_config.pipeline_substitutions.clear();
            for(auto& r : g_pipeline.get_steps()) {
                if(r->type == url_pipeline_step_type::find_replace) {
                    auto rr = std::static_pointer_cast<bt::pipeline::replacer>(r);
                    g_config.pipeline_substitutions.push_back(rr->serialise());
                }
            }

            // recompute
            url_subs_up.clear(true);
            g_pipeline.process(url_subs_up);
            recompute = false;
        }

    }

    void config_app::render_dashboard() {

        w::guard gpop{pop_dash};
        if(!pop_dash) return;

        bool recheck{false};
        for(auto& sc : health_checks) {
            string tooltip = sc.description;
            {
                w::group g;
                g.render();

                w::label(sc.is_ok ? ICON_MD_DONE : ICON_MD_ERROR, sc.is_ok ? w::emphasis::primary : w::emphasis::error);
                w::sl();
                w::label(sc.name);

                if(!sc.is_ok) {
                    w::sl(250 * app->scale);
                    if(w::button("fix", w::emphasis::error, true, true)) {
                        sc.fix();
                        recheck = true;
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

        if(w::button("recheck", w::emphasis::primary) || recheck) {
            check_health();
            recheck = false;
        }
    }

    void config_app::render_scripting_window() {
        w::guard gw{wnd_scripting};

        if(!script_initialised) {
            script_editor.set_text(g_script.get_code());
            script_initialised = true;
        }

        if(!g_script.get_error().empty()) {
            w::container c{"err", 0, 0};
            c.border().auto_size_y();
            w::guard g{c};
            w::label(g_script.get_error(), w::emphasis::error);
        }

        w::combo("##fn", g_script.bt_function_names, script_fn_selected, 250);
        string func_name = g_script.bt_function_names.empty() ? "" : g_script.bt_function_names[script_fn_selected];
        bool is_ppl = func_name.starts_with(LuaPipelinePrefix);
        w::tooltip("function to execute");

        w::sl();
        if(w::button(ICON_MD_PLAY_ARROW, w::emphasis::primary)) {
            g_script.set_code(script_editor.get_text());

            if(g_script.get_error().empty()) {
                // test it
                script_terminal += fmt::format("{}\nExecuting '{}'...\n", datetime::to_iso_8601(), func_name);

                click_payload up;
                up.url = g_config.pv_last_url;
                up.window_title = g_config.pv_last_wt;
                up.process_name = g_config.pv_last_pn;

                if(is_ppl) {
                    string out_url = g_script.call_ppl(up, func_name);
                    script_terminal += g_script.print_buffer;
                    script_terminal += fmt::format("result: {}\n------------\n", out_url);
                } else {

                    g_pipeline.process(up);
                    if(up.url != g_config.pv_last_url) {
                        script_terminal += fmt::format("pipeline changed URL to '{}'\n", up.url);
                    }

                    g_script.print_buffer.clear();
                    bool matched = g_script.call_rule(up, func_name);
                    script_terminal += g_script.print_buffer;

                    script_terminal += fmt::format("rule match: {}\n------------\n", matched);
                }
            }
        }
        w::tooltip("save and run");

        // test input
        if(!func_name.empty()) {
            w::input(g_config.pv_last_url, ICON_MD_LINK " URL", true);
            if(!is_ppl) {
                w::input(g_config.pv_last_wt, ICON_MD_WINDOW " window", true);
                w::input(g_config.pv_last_pn, ICON_MD_MEMORY " process", true);
            }
        }

        float terminal_height = 400 * app->scale;

        w::sep(strings::LuaScript);
        script_editor.render(0, ImGui::GetWindowHeight() - terminal_height);

        w::sep("Terminal");
        if(w::button(ICON_MD_CLEAR_ALL, w::emphasis::error)) {
            script_terminal.clear();
        }
        w::tooltip("clear");
        w::sl();
        w::icon_checkbox(ICON_MD_ARROW_DOWNWARD, script_terminal_autoscroll);
        w::tooltip("auto-scroll");
        w::input_ml("##script_terminal", script_terminal, -FLT_MIN, script_terminal_autoscroll);
    }

    void config_app::render_pipe_visualiser_window() {
        w::guard gw{wnd_pv};

        bool i0 = w::input(g_config.pv_last_url, ICON_MD_LINK " URL");
        bool i1 = w::input(g_config.pv_last_wt, ICON_MD_WINDOW " window", true, 300.0f * app->scale);
        w::sl();
        bool i2 = w::input(g_config.pv_last_pn, ICON_MD_MEMORY " process", true, 150.0f * app->scale);

        if(w::button(ICON_MD_CLEAR_ALL " clear", w::emphasis::error)) {
            g_config.pv_last_url = g_config.pv_last_wt = g_config.pv_last_pn = "";
        }

        w::sl();
        bool refresh = w::button("refresh") || pv_pipeline_steps.size() != g_pipeline.get_steps().size();
        w::tooltip("Refresh calculations");

        w::sl();
        w::checkbox("matching only", pv_only_matching);

        if(pv_pipeline_steps.empty() || refresh || i0 || i1 || i2) {
            pv_cp.clear();
            pv_cp.url = g_config.pv_last_url;
            pv_cp.window_title = g_config.pv_last_wt;
            pv_cp.process_name = g_config.pv_last_pn;
            pv_pipeline_steps = g_pipeline.process_debug(pv_cp);

        }

        // do it continuously
        recalculate_test_url_matches(pv_cp);


        if(ImGui::BeginTable("pv", 3,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_NoBordersInBodyUntilResize | 
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_HighlightHoveredColumn |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY)) {

            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            // input row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            w::label("URL");
            ImGui::TableSetColumnIndex(1);
            w::label(g_config.pv_last_url);

            // pipeline steps
            if(!pv_pipeline_steps.empty()) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                bool open = ImGui::TreeNodeEx("Pipeline",
                    ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled(" ");

                if(open) {
                    for(auto& s : pv_pipeline_steps) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        string text = url_pipeline_step::to_string(s.step->type);
                        ImGui::TreeNodeEx(text.c_str(), ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                        ImGui::TableSetColumnIndex(1);
                        if(s.before.url == s.after.url) {
                            w::label(ICON_MD_BRIGHTNESS_1);
                            w::tooltip("no change");
                        } else {
                            w::label(ICON_MD_ADJUST, w::emphasis::primary);
                            w::tooltip("URL was modified");
                        }
                        w::sl();
                        w::label(s.after.url);
                    }
                    ImGui::TreePop();
                }
            }

            // browsers?
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            bool open = ImGui::TreeNodeEx("Browsers", ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled(" ");
            if(open) {
                for(auto b : g_config.browsers) {
                    if(pv_only_matching && !b->ui_test_url_matches) continue;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    bool b_open = w::tree_node(b->name,
                        ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen,
                        b->ui_test_url_matches ? w::emphasis::primary : w::emphasis::none);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextDisabled(" ");
                    if(b_open) {

                        for(auto i : b->instances) {
                            if(pv_only_matching && !i->ui_test_url_matches) continue;
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            auto emp = i->ui_test_url_matches ? w::emphasis::primary : w::emphasis::none;
                            bool i_open = w::tree_node(i->name,
                                ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen, emp);
                            ImGui::TableSetColumnIndex(1);

                            if(i->rules.empty()) {
                                w::label("no rules", 0, false);
                            } else {
                                w::label(fmt::format("{} rule(s)", i->rules.size()), 0, false);
                            }

                            if(i_open) {

                                // rules
                                for(auto r : i->rules) {
                                    if(pv_only_matching && !r->ui_test_url_matches) continue;
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    auto emp = r->ui_test_url_matches ? w::emphasis::primary : w::emphasis::none;
                                    w::tree_node(
                                        r->get_type_string(),
                                        ImGuiTreeNodeFlags_SpanAllColumns |
                                        ImGuiTreeNodeFlags_Leaf |
                                        ImGuiTreeNodeFlags_NoTreePushOnOpen,
                                        emp);
                                    ImGui::TableSetColumnIndex(1);
                                    w::label(r->to_string(false), emp);
                                }


                                ImGui::TreePop();
                            }
                        }

                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();   // browsers
            }
            ImGui::EndTable();
        }
    }
    
    void config_app::render_status_bar() {
        w::status_bar sb;

        w::label(ICON_MD_HEALTH_AND_SAFETY, health_failed > 0 ? w::emphasis::error : w::emphasis::primary);
        w::tooltip("Shows if there are any issues preventing " APP_SHORT_NAME " from working properly");
        if(ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        // re-open health dashboard periodically if there are issues
        health_frame_time += ImGui::GetIO().DeltaTime;
        if(health_frame_time >= 20.0f) {
            health_frame_time = 0;

            if(health_failed > 0) {
                ImVec2 pos = ImGui::GetItemRectMin();
                pop_dash.open(pos.x, pos.y);
            }
        }

        if(w::is_leftclicked()) {
            pop_dash.open();
        }

        size_t ipc{0};
        size_t irc{0};
        for(const auto& b : g_config.browsers) {
            ipc += b->instances.size();
            for(const auto& i : b->instances) {
                irc += i->rules.size();
            }
        }

        w::sl();
        w::label("|", 0, false);
        w::sl();
        w::label(fmt::format("{} {}", ICON_MD_WEB, g_config.browsers.size()), 0, false);
        w::tooltip("Browser count");

        w::sl();
        w::label(fmt::format("{} {}", ICON_MD_PERSON, ipc), 0, false);
        w::tooltip("Profile count");

        w::sl();
        w::label(fmt::format("{} {}", ICON_MD_RULE, irc), 0, false);
        w::tooltip("Configured rule count");

        w::sl();
        w::label("|", 0, false);

        w::sl();
        w::label(ICON_MD_COFFEE, 0, false);
        w::tooltip("Support this app, buy me a coffee!");
        if(w::is_hovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if(w::is_leftclicked()) {
            url_opener::open(APP_BUYMEACOFFEE_URL);
        };

        if(!g_config.browsers.empty()) {
            w::sl(); w::label("|", 0, false);
            const auto dbr = browser::get_default(g_config.browsers, g_config.default_profile_long_id);
            w::sl(); w::label(ICON_MD_LAPTOP, 0, false);
            w::sl(); w::label(dbr->b->name, 0, false);
            w::tooltip("Default browser");

            if(dbr->b->is_system) {
                w::sl(); w::label("|", 0, false);
                w::sl(); w::label(ICON_MD_TAB, 0, false);
                w::sl(); w::label(dbr->name, 0, false);
                w::tooltip("Default profile");
            }
        }
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
            rediscover_browsers();
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
            w::icon_checkbox(ICON_MD_VISIBILITY, g_config.show_hidden_browsers);
            w::tooltip("Show hidden browsers");

            for(int i = 0; i < g_config.browsers.size(); i++) {
                auto br = g_config.browsers[i];
                if(!g_config.show_hidden_browsers && br->is_hidden) {
                    continue;
                }

                render_card(br, i == selected_browser_idx);

                // we can now react on click
                if(w::is_leftclicked()) {
                    selected_browser_idx = i;
                }
            }
        }
        w::sl();
        {
            w::guard g{w_right_panel};

            if(selected_browser_idx < g_config.browsers.size()) {
                render_detail(g_config.browsers[selected_browser_idx]);
            }
        }
    }

    std::shared_ptr<bt::browser_instance> config_app::get_selected_browser_instance() {
        auto browser = g_config.browsers[selected_browser_idx];
        if(browser->is_system) {
            return browser->instances[selected_profile_idx];
        } else {
            return browser->instances[0];
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

        string path = b->get_best_icon_path();
        if(app->preload_texture(path, path)) {
            w::image(*app, path, icon_size, icon_size);
        } else {
            w::image(*app, "logo", icon_size, icon_size);
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
                    w::icon_image(*app, "bt_chromium");
                    w::tooltip(strings::ChromiumBased);
                } else if(b->is_firefox) {
                    w::sl();
                    w::icon_image(*app, "bt_gecko");
                    w::tooltip(strings::GeckoBased);
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
                w::label(ICON_MD_VISIBILITY_OFF, 0, false);
                w::tooltip("Hidden");
            }

            if(b->contains_profile_id(g_config.default_profile_long_id)) {
                w::sl();
                w::label(ICON_MD_FAVORITE, w::emphasis::primary);
                w::tooltip("Default browser");
            }

        } else {
            w::label(ICON_MD_FACE, 0, false);
            w::tooltip("no profiles");
        }

        w::spc();
        w::set_pos(0, -1);
    }

    void config_app::render_detail(std::shared_ptr<bt::browser> b) {

        // begin toolbar

        // hide/show button rendered as a button due to wrong looks if rendered as a checkbox
        w::sl();
        if(b->is_hidden) {
            if(w::button(ICON_MD_VISIBILITY)) {
                b->is_hidden = false;
            }
            w::tooltip("Show this browser in the browser list");
        } else {
            if(w::button(ICON_MD_VISIBILITY_OFF)) {
                b->is_hidden = true;
            }
            w::tooltip("Hide this browser from the browser list");
        }

        if(!b->open_cmd.empty()) {
            w::sl();
            if(w::button(ICON_MD_FOLDER)) {
                std::filesystem::path p{b->open_cmd};
                string path = p.parent_path().string();
                win32::shell::exec(path, "");
            }
            w::tooltip(fmt::format("open {}'s folder in Explorer.", b->name));
        }

        if(b->is_system) {
            // anything?
        } else {
            w::sl();
            if(w::button(ICON_MD_FAVORITE)) {
                g_config.default_profile_long_id = b->instances[0]->long_id();
            }
            w::tooltip("Make this browser the default one");

            w::sl();
            if(w::button(ICON_MD_LAUNCH)) {
                url_opener::open(b->instances[0], APP_URL);
            }
            w::tooltip("test by opening a link");

            w::sl();
            if(w::button(ICON_MD_DELETE " delete", w::emphasis::error)) {
                size_t idx = browser::index_of(g_config.browsers, b);

                // erase and save
                std::erase_if(g_config.browsers, [b](auto i) { return i->id == b->id; });

                // if possible, select previous browser
                if(idx != string::npos) {
                    idx -= 1;
                    if(idx >= 0 && idx < g_config.browsers.size()) {
                        selected_browser_idx = idx;
                    }
                }
            }
            w::tooltip("Completely deletes this browser, no questions asked");
        }

        // --- toolbar end


        // --- profiles start

        if(b->is_system) {
            w::tab_bar tabs{b->id};

            int idx{0};
            for(shared_ptr<browser_instance> bi : b->instances) {
                string tab_icon;
                if(bi->is_incognito) {
                    tab_icon = fmt::format("{} ", ICON_MD_SECURITY);
                }
                string tab_title = fmt::format(" {}{} ", tab_icon, bi->name);

                {
                    auto t = tabs.next_tab(tab_title);
                    if(t) {
                        selected_profile_idx = idx;
                        w::spc();

                        // mini toolbar

                        if(bi->long_id() == g_config.default_profile_long_id) {
                            w::button(ICON_MD_FAVORITE, w::emphasis::none, false);
                        }
                        else if(w::button(ICON_MD_FAVORITE, w::emphasis::primary)) {
                            g_config.default_profile_long_id = bi->long_id();
                        }
                        w::tooltip("Make this browser the default one");

                        w::sl();
                        if(w::button(ICON_MD_LAUNCH)) {
                            url_opener::open(bi, APP_URL);
                        }
                        w::tooltip("test by opening a link");

                        if(b->is_chromium) {
                            w::sl();
                            if(w::button(ICON_MD_TAB_UNSELECTED)) {
                                click_payload up{APP_URL};
                                up.app_mode = true;
                                url_opener::open(bi, up);
                            }
                            w::tooltip("test by opening a link as an app");
                        }

                        if(!b->open_cmd.empty()) {
                            if(b->is_firefox) {

                                if(g_config.firefox_mode == firefox_container_mode::off) {
                                    w::sl();
                                    if(w::button(ICON_MD_SUPERVISOR_ACCOUNT)) {
                                        win32::shell::exec(b->open_cmd, "-P");
                                    }
                                    w::tooltip("open Firefox Profile Manager (-P flag)");

                                    w::sl();
                                    if(w::button(ICON_MD_SUPERVISED_USER_CIRCLE)) {
                                        win32::shell::exec(b->open_cmd, "about:profiles");
                                    }
                                    w::tooltip("open Firefox Profile Manager in Firefox itself");
                                } else {
                                    w::sl();
                                    if(w::button(ICON_MD_EXTENSION, w::emphasis::error)) {
                                        url_opener::open("https://aloneguid.github.io/bt/firefox-containers.html");
                                    }
                                    w::tooltip("download required extension");
                                }
                            } else if(b->is_chromium) {
                                w::sl();
                                if(w::button(ICON_MD_EXTENSION)) {
                                    // extenstion page needs to be opened in the correct profile
                                    url_opener::open(bi, APP_BROWSER_EXTENSIONS_DOCS_URL);
                                }
                                w::tooltip("download optional integration extension");
                            }
                        }

                        // end of mini toolbar

                        render_icon(bi->get_best_icon_path(false), bi->is_incognito, bi->user_icon_path);
                         
                        w::sl();
                        {
                            w::group g;
                            g.render();

                            w::input(bi->b->open_cmd, "cmd", true, 0, true);;
                            w::tooltip("Location");

                            w::input(bi->launch_arg, "arg", true, 0, true);
                            w::tooltip("Discovered arguments (read-only)");

                            if(!bi->b->is_msstore()) {
                                w::input(bi->user_arg, "extra arg");
                                w::tooltip("Any extra arguments to pass.\nIf you break it, you fix it ;)");
                            }
                        }

                        w::spc();
                        render_rules(bi);
                    }
                    idx++;
                }
            }
        } else {
            shared_ptr<browser_instance> bi = b->instances[0];

            render_icon(bi->get_best_icon_path(false), bi->is_incognito, bi->user_icon_path);

            w::sl();
            {
                w::group g;
                g.render();

                w::input(b->open_cmd, "exe", false);
                w::tooltip("Full path to browser executable. The only way to change this is to re-create the browser. Sorry ;)");

                if(w::input(bi->name, "name")) {
                    b->name = bi->name;
                }

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

    void config_app::render_icon(const std::string& path_default, bool is_incognito, string& path_override) {
        {
            w::group g;
            g
                //.border(ImGuiCol_Border)
                .border_hover(ImGuiCol_ButtonHovered)
                .render();

            float box_size = 40 * app->scale;
            string final_path;

            if(is_incognito) {
                if(!path_override.empty() && app->preload_texture(path_override, path_override)) {
                    w::rounded_image(*app, path_override, box_size - 1, box_size - 1, box_size / 2);
                    final_path = path_override;
                } else {
                    w::rounded_image(*app, "incognito", box_size - 1, box_size - 1, box_size / 2);
                }
            } else {
                string path = path_override.empty() ? path_default : path_override;
                final_path = path;

                if(!path.empty() && app->preload_texture(path, path)) {
                    w::rounded_image(*app, path, box_size - 1, box_size - 1, box_size / 2);
                } else {
                    ImGui::Dummy(ImVec2(box_size, box_size));
                }
            }
        }
        if(w::is_hovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            w::tooltip("Click to override the built-in icon\nRight-click to reset to default");
        }

        if(w::is_leftclicked()) {
            string icon_path = win32::shell::file_open_dialog("PNG", "*.png");
            if(!icon_path.empty()) {
                path_override = icon_path;
            }
        }
        if(w::is_rightclicked()) {
            path_override.clear();
        }
    }

    void config_app::render_rules(std::shared_ptr<browser_instance> bi) {
        //w::label("Rules");
        w::sep("Rules");

        if(w::button(ICON_MD_ADD " add", w::emphasis::primary)) {
            bi->add_rule(fmt::format("rule {}", bi->rules.size()));
        }

        w::sl();
        if(w::button(ICON_MD_DELETE " clear all", w::emphasis::error)) {
            bi->rules.clear();
        }

        w::sl(); ww::help_link("rules.html");

        // scrollable area with list of rules
        {
            w::container c{"rules"};
            w::guard g{c};

            for(int i = 0; i < bi->rules.size(); i++) {
                auto rule = bi->rules[i];

                // location
                w::combo(string{"##loc"} + std::to_string(i), 
                    rule_locations, (size_t&)rule->loc, 90);

                // value
                w::sl();
                string val_label = string{"##val"} + std::to_string(i);
                if(rule->loc == match_location::lua_script) {

                    // get selected index
                    size_t selected{0};
                    for(size_t j = 0; j < g_script.rule_function_names.size(); j++) {
                        if(g_script.rule_function_names[j] == rule->value) {
                            selected = j;
                            break;
                        }
                    }

                    w::combo(val_label, g_script.rule_function_names, selected, 250);
                    w::tooltip(strings::LuaScriptTooltip);

                    // reassign value
                    if(!g_script.rule_function_names.empty()) {
                        rule->value = g_script.rule_function_names[selected];
                    }

                } else {
                    w::input(rule->value, val_label, true, 250 * app->scale);
                }

                // up/down logic is very custom and is bound to the textbox itself
                if(ImGui::IsItemFocused()) {
                    if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                        stl::move(bi->rules, i, -1, true);
                    } else if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                        stl::move(bi->rules, i, 1, true);
                    }
                }

                // is regex checkbox (not for Lua)
                if(rule->loc != match_location::lua_script) {
                    w::sl();
                    w::icon_checkbox(ICON_MD_GRAIN, rule->is_regex);
                    w::tooltip(strings::RuleIsARegex);
                }

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
                if(w::button(string{ICON_MD_DELETE} + "##" + to_string(i), w::emphasis::error)) {
                    bi->delete_rule(rule->value);
                }
                w::tooltip("Delete rule");
            }
        }
    }

    void config_app::rediscover_browsers() {
        vector<shared_ptr<browser>> fresh_browsers = discovery::discover_all_browsers();
        fresh_browsers = browser::merge(fresh_browsers, g_config.browsers);
        g_config.browsers = fresh_browsers;

        string message = fmt::format("Discovered {} browser(s).", g_config.browsers.size());
        w::notify_info(message);
    }

    void config_app::add_custom_browser_by_asking() {
        string exe_path = win32::shell::file_open_dialog("Windows Executable", "*.exe");
        if(exe_path.empty()) return;

        wstring w_product_name = win32::user::get_file_version_info_string(exe_path, "ProductName");
        string id = win32::ole32::create_guid();
        string name = str::to_str(w_product_name);
        auto b = make_shared<browser>(id, name, exe_path, false);
        b->instances.push_back(make_shared<browser_instance>(b, "default", name, "", ""));

        g_config.browsers.push_back(b);

        // find this new browser and select it (it won't be the last in the list)
        size_t idx = browser::index_of(g_config.browsers, b);
        if(idx != string::npos) {
            selected_browser_idx = idx;
        }
    }

    void config_app::recalculate_test_url_matches(const click_payload& cp) {
        for(auto b : g_config.browsers) {
            b->ui_test_url_matches = false;
            for(auto bi : b->instances) {
                bi->ui_test_url_matches = false;

                for(auto r : bi->rules) {
                    r->ui_test_url_matches = false;
                    if(r->is_match(cp, g_script)) {
                        r->ui_test_url_matches = true;
                        bi->ui_test_url_matches = true;
                        b->ui_test_url_matches = true;
                    }
                }
            }
        }
    }
}