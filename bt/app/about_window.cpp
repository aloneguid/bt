#include "about_window.h"
#include <fmt/core.h>
#include "../globals.h"
#include "../res.h"
#include "ui.h"

using namespace std;
using namespace grey;

namespace bt {
    about_window::about_window(grey::grey_context& ctx) : grey::window{ctx, "About", 310, 290} {
        can_resize = false;
        float scale = get_system_scale();

        float icon_size = 50 * scale;
        set_pos(width / 2 - icon_size / 2, 40 * scale);
        make_image_from_memory("logo", icon_png, icon_png_len,
            icon_size,
            icon_size);
        spacer(); spacer(); spacer();

        auto about_txt = make_label(R"(Browser Tamer acts as a virtual browser, but instead of showing web pages, it redirects links to a browser or browser profile of your choice.

It super fast, extremely light on resources, completely free and open source.)");
        about_txt->text_wrap_pos = 300 * scale;
        spacer();

        make_label("Version:"); 
        
        same_line(); make_label(APP_VERSION)->set_emphasis(emphasis::primary);

        make_label("ImGui Version:");
        same_line(); make_label(ImGui::GetVersion())->set_emphasis(emphasis::primary);
        same_line(); make_label("(docking branch)");

        separator();

        auto cmd_homepage = make_button("Homepage");
        cmd_homepage->on_pressed = [this](button&) {
            ui::url_open(url_payload{HomeUrl, "", "ui_about_home"}, ui::open_method::configured);
        };

        same_line();
        make_button("GitHub")->on_pressed = [](button&) {
            ui::url_open(url_payload{APP_GITHUB_URL, "", "ui_about_github"}, ui::open_method::configured);
        };

        same_line();
        auto cmd_close = make_button("Close");
        cmd_close->on_pressed = [this](button&) {
            is_visible = false;
        };
    }
}