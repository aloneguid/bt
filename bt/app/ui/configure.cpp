#include "configure.h"

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
        w::window wnd{"About"};
        wnd
            .size(310, 350, app->scale)
            .no_resize()
            .render();
    }
}