#include "configure.h"
#include "../../res.inl"
#include "fmt/core.h"
#include "win32/process.h"
#include "win32/shell.h"
#include "win32/sysinfo.h"
#include "str.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    config_app::config_app() {
        title = string{APP_LONG_NAME} + " " + APP_VERSION;
        app = grey::app::make(title);
    }

    void config_app::run() {
        app->run([this](const grey::app& app) { return run_frame(); });
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

#if _DEBUG
        if(show_demo) {
            ImGui::ShowDemoWindow();
        }
#endif

        if(show_about)
            run_about_window_frame();

        return is_open;
    }

    void config_app::handle_menu_click(const std::string& id) {
        if(id == "demo") {
            show_demo = !show_demo;
        } else if(id.starts_with("set_theme")) {
            grey::themes::set_theme(id, app->scale);
        } else if(id == "?") {
            show_about = !show_about;
        }
    }

    void config_app::run_about_window_frame() {

        app->preload_texture("logo", icon_png, icon_png_len);

        const int width = 310;
        w::window wnd{"About"};
        wnd
            .size(width, 350, app->scale)
            .no_resize()
            .render();

        float icon_size = 50 * app->scale;
        w::set_pos(width * app->scale / 2 - icon_size / 2, 40 * app->scale);
        w::image(*app, "logo", icon_size, icon_size);
        w::sp(3);

        w::label(
            R"(Browser Tamer acts as a virtual browser, but instead of showing web pages, it redirects links to a browser or browser profile of your choice.

It super fast, extremely light on resources, completely free and open source.)",
300 * app->scale);
        w::sp();

        w::label("Version:");

        w::sl(); w::label(APP_VERSION);// ->set_emphasis(emphasis::primary);

        w::label("ImGui Version:");
        w::sl(); w::label(ImGui::GetVersion());// ->set_emphasis(emphasis::primary);
        w::sl(); w::label("(docking branch)");

        // sys info

        // refresh sys info
        {
            about_frame_time += ImGui::GetIO().DeltaTime;
            if(about_frame_time >= 1) {
                about_frame_time = 0;

                about_fps = fmt::format("{} {:.1f}", ICON_FK_FILM, ImGui::GetIO().Framerate);
                about_fps_tooltip = fmt::format(
                    "Framerate: {:.1f}\nScale: {:.1f}\nDPI: {}",
                    ImGui::GetIO().Framerate,
                    app->scale,
                    win32::shell::get_dpi());

                // memory usage
                win32::process p;
                uint64_t working_set;
                if(p.get_memory_info(working_set)) {
                    about_mem = fmt::format("{} {}", ICON_FK_MICROCHIP, str::to_human_readable_size(working_set));
                }

                // total CPU usage
                about_cpu = fmt::format("{} {:.1f}", ICON_FK_MICROCHIP, win32::system_info::get_cpu_usage_perc());
            }
        }

        w::sp();

        w::label(about_fps);
        w::tooltip(about_fps_tooltip);

        w::label(about_mem);
        w::tooltip("Process memory usage (when this dialog is closed, usage goes down massively)");

        w::label(about_cpu);
        w::tooltip("Current CPU load of your entire system (not this application) in percentages.");

        w::sp();

        /*auto cmd_homepage = make_button(ICON_FK_HOME " Home");
        cmd_homepage->on_pressed = [this](button&) {
            ui::url_open(url_payload{APP_URL}, ui::open_method::configured);
        };

        same_line();
        make_button(ICON_FK_GITHUB " GitHub")->on_pressed = [](button&) {
            ui::url_open(url_payload{APP_GITHUB_URL}, ui::open_method::configured);
        };

        same_line();
        auto cmd_close = make_button("Close");
        cmd_close->on_pressed = [this](button&) {
            close();
        };

        on_frame = [this](grey::component& c) {
            this->tag_float += ImGui::GetIO().DeltaTime;

            if(this->tag_float >= 1) {
                refresh_system_status();

                this->tag_float = 0;
            }
        };

        refresh_system_status();*/
    }
}