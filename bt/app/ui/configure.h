#pragma once
#include "app.h"
#include "widgets.h"
#include <string>
#include <memory>
#include <map>
#include "fonts/forkawesome.h"
#include "../browser.h"
#include "../setup.h"
#include "url.h"

namespace bt::ui {

    struct rule_match_status {
        std::string id;
        bool matched;
        size_t version;
    };

    class config_app {
    public:
        config_app();
        ~config_app();
        void run();

    private:

        std::unique_ptr<grey::app> app;
        std::string title;
        grey::widgets::window wnd_config;
        grey::widgets::popup pop_dash{"pop_dash"};
        grey::widgets::window wnd_about;
        bool is_open{true};
        std::vector<std::shared_ptr<bt::browser>> browsers;
        std::map<std::string, rule_match_status> id_to_rule_match_status;
#if _DEBUG
        bool show_demo{false};
#endif
        // UI elements
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
        std::vector<system_check> health_checks;
        size_t health_succeeded{0};
        size_t health_failed{0};
        void check_health();

        // URL Tester
        size_t url_tester_payload_version{0};
        url_payload url_tester_up;

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
        void render_dashboard();
        void render_url_tester_input();

        void render_status_bar();
        void render_no_browsers();
        void render_browsers();
        void render_card(std::shared_ptr<bt::browser> b, bool is_selected);
        void render_detail(std::shared_ptr<bt::browser> b);
        void render_rules(std::shared_ptr<browser_instance> bi);

        void rediscover_browsers();
        void add_custom_browser_by_asking();
        void test_url();

        bool matches_test_url(std::shared_ptr<bt::browser> b);
    };
}