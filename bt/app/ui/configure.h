#pragma once
#include "app.h"
#include "widgets.h"
#include <string>
#include <memory>

namespace bt::ui {
    class config_app {
    public:
        config_app();
        void run();

    private:

        std::unique_ptr<grey::app> app;
        std::string title;
        bool is_open{true};
#if _DEBUG
        bool show_demo{false};
#endif

        // "About" window
        bool show_about{false};
        float about_frame_time{5.0f};
        std::string about_fps;
        std::string about_fps_tooltip;
        std::string about_mem;
        std::string about_cpu;

        std::vector<grey::widgets::menu_item> menu_items
        {
            { "File", {
                { "+b", "Add Custom Browser", ICON_FK_PLUS_CIRCLE },
                { "config.ini", {
                    { "ini", "Open" },
                    { "ini+c", "Copy path to clipboard" } },
                    ICON_FK_COGS },
                { "hit_log.csv", {
                    { "csv", "Open" },
                    { "csv+c", "Copy path to clipboard" } },
                    ICON_FK_BOOK },
                { "", "-" },
                { "x", "Exit", ICON_FK_SIGN_OUT }
            } },
            { "Tools", {
                { "dash", "Readiness dashboard", ICON_FK_TACHOMETER },
                { "test", "URL Tester", ICON_FK_LAPTOP },
                { "windows_defaults", "Windows Defaults", ICON_FK_WINDOWS },
                { "refresh", "Rediscover Browsers", ICON_FK_RETWEET },
                { "open_picker", "Test URL Picker", ICON_FK_CROSSHAIRS },
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
                        { "mi_phk_cs", "Ctrl+Shift+" ICON_FK_MOUSE_POINTER },
                        { "mi_phk_ca", "Ctrl+Alt+" ICON_FK_MOUSE_POINTER },
                        { "mi_phk_as", "Alt+Shift+" ICON_FK_MOUSE_POINTER }
                    } }
                } },
                { "Firefox container mode", {
                    { "mi_ff_mode_off", "off (use profiles)", ICON_FK_POWER_OFF },
                    { "mi_ff_mode_bt", APP_LONG_NAME, ICON_FK_PUZZLE_PIECE },
                    { "mi_ff_mode_ouic", "open-url-in-container", ICON_FK_PUZZLE_PIECE }
                    }, ICON_FK_FIREFOX },
                { "Theme", grey::widgets::menu_item::make_ui_theme_items(), ICON_FK_PAINT_BRUSH },
                { "log_rule_hits", "Log Rule Hits to File" },
                { "", "-" },
                { "pipeline_config", "Configure URL pipeline", ICON_FK_BOLT }
            } },
            { "Help", {
                { "browser_ex", "Extensions", ICON_FK_PUZZLE_PIECE },
                { "contact", "Contact", ICON_FK_ENVELOPE },
                { "releases", "All Releases" },
                { "check_version", "Check for Updates" },
                { "Registry", {
                    { "reg_xbt", "Custom Protocol" },
                    { "reg_browser", "Browser Registration" }
                }},
                { "", "-" },
                { "doc", "Documentation", ICON_FK_BOOK },
                { "?", "About", ICON_FK_INFO }
#if _DEBUG
                ,
                { "", "-" },
                { "demo", "Demo", ICON_FK_MAGIC }
#endif
            }}
        };

        bool run_frame();
        void handle_menu_click(const std::string& id);

        void run_about_window_frame();
    };
}