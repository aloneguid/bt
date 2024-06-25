#pragma once
#include "app.h"
#include "widgets.h"
#include <string>
#include <memory>
#include "fonts/forkawesome.h"
#include "../browser.h"
#include "../setup.h"

namespace bt::ui {
    class config_app {
    public:
        config_app();
        void run();

    private:

        std::unique_ptr<grey::app> app;
        std::string title;
        bool is_open{true};
        std::vector<std::shared_ptr<bt::browser>> browsers;
#if _DEBUG
        bool show_demo{false};
#endif
        // UI elements
        bool show_hidden_browsers{true};
        size_t selected_browser_idx{0};
        grey::widgets::container w_left_panel;
        grey::widgets::container w_right_panel;

        // "About" window
        bool show_about{false};
        float about_frame_time{5.0f};
        std::string about_fps;
        std::string about_fps_tooltip;
        std::string about_mem;
        std::string about_cpu;

        // "Health Dashboard" window
        bool show_dash{false};
        std::vector<system_check> health_checks;
        size_t health_succeeded{0};
        size_t health_failed{0};
        void check_health();

        std::vector<grey::widgets::menu_item> menu_items
        {
            { "File", {
                { "+b", "Add Custom Browser", ICON_MD_ADD_CIRCLE },
                { "config.ini", {
                    { "ini", "Open" },
                    { "ini+c", "Copy path to clipboard" } },
                    ICON_MD_SETTINGS },
                { "hit_log.csv", {
                    { "csv", "Open" },
                    { "csv+c", "Copy path to clipboard" } },
                    ICON_MD_MENU_BOOK },
                { "", "-" },
                { "x", "Exit", ICON_MD_LOGOUT }
            } },
            { "Tools", {
                { "dash", "Readiness dashboard", ICON_MD_DASHBOARD },
                { "test", "URL Tester", ICON_MD_CRUELTY_FREE },
                { "windows_defaults", "Windows Defaults", ICON_MD_PSYCHOLOGY },
                { "refresh", "Rediscover Browsers", ICON_MD_REFRESH },
                { "open_picker", "Test URL Picker", ICON_MD_CRUELTY_FREE },
                { "Troubleshooting", {
                    { "fix_xbt", "Re-register Custom Protocol" },
                    { "fix_browser", "Re-register as Browser" } }
                }
            } },
            { "Settings", {
                { "Browser Picker Mode", {
                    { "open_method_silent", "Never Ask" },
                    { "open_method_decide", "Ask on Conflict" },
                    { "open_method_pick", "Always Ask" },
                    { "Also Open On", {
                        { "mi_phk_never", "Never" },
                        { "mi_phk_cs", "Ctrl+Shift+" ICON_MD_MOUSE },
                        { "mi_phk_ca", "Ctrl+Alt+" ICON_MD_MOUSE },
                        { "mi_phk_as", "Alt+Shift+" ICON_MD_MOUSE }
                    } }
                } },
                { "Firefox container mode", {
                    { "mi_ff_mode_off", "off (use profiles)" },
                    { "mi_ff_mode_bt", APP_LONG_NAME, ICON_MD_EXTENSION },
                    { "mi_ff_mode_ouic", "open-url-in-container", ICON_MD_EXTENSION }
                    }, ICON_MD_LOCAL_FIRE_DEPARTMENT },
                { "Theme", grey::widgets::menu_item::make_ui_theme_items(), ICON_MD_DARK_MODE },
                { "log_rule_hits", "Log Rule Hits to File" },
                { "", "-" },
                { "pipeline_config", "Configure URL pipeline" }
            } },
            { "Help", {
                { "browser_ex", "Extensions", ICON_MD_EXTENSION },
                { "contact", "Contact" },
                { "releases", "All Releases" },
                { "check_version", "Check for Updates" },
                { "Registry", {
                    { "reg_xbt", "Custom Protocol" },
                    { "reg_browser", "Browser Registration" }
                }},
                { "", "-" },
                { "doc", "Documentation" },
                { "?", "About" }
#if _DEBUG
                ,
                { "", "-" },
                { "demo", "Demo", ICON_MD_SLIDESHOW }
#endif
            }}
        };
        std::vector<std::string> rule_locations { "URL", "Title", "Process" };
        std::vector<std::pair<std::string, std::string>> url_scopes{
            { ICON_MD_LANGUAGE, "Match anywhere" },
            { ICON_MD_GITE, "Match only in host name" },
            { ICON_MD_ROUNDABOUT_LEFT, "Match only in path" }
        };

        bool run_frame();
        void handle_menu_click(const std::string& id);

        void render_about_window();
        void render_dashboard_window();

        void render_status_bar();
        void render_no_browsers();
        void render_browsers();
        void render_card(std::shared_ptr<bt::browser> b, bool is_selected);
        void render_detail(std::shared_ptr<bt::browser> b);
        void render_rules(std::shared_ptr<browser_instance> bi);

        void add_custom_browser_by_asking();
    };
}