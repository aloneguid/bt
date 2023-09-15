#include "about_window.h"
#include <fmt/core.h>
#include "../globals.h"
#include "../res.inl"
#include "ui.h"
#include <fmt/core.h>
#include "win32/shell.h"
#include "win32/sysinfo.h"
#include "win32/process.h"
#include "str.h"

using namespace std;
using namespace grey;

namespace bt {
    about_window::about_window(grey::grey_context& ctx) : grey::window{ctx, "About", 310, 350}, ctx{ctx} {
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

        // sys info
        spacer();

        st_fps = make_label("");

        auto lbl = make_label(&mem_display);
        lbl->tooltip = "Process memory usage (when this dialog is closed, usage goes down massively)";

        auto st_cpu = make_label(&cpu_display);
        st_cpu->tooltip = "Current CPU load of your system in percentages.";

        separator();

        auto cmd_homepage = make_button(ICON_FA_HOUSE " Home");
        cmd_homepage->on_pressed = [this](button&) {
            ui::url_open(url_payload{APP_URL}, ui::open_method::configured);
        };

        same_line();
        make_button(ICON_FA_GITHUB " GitHub")->on_pressed = [](button&) {
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

        refresh_system_status();
    }

    void about_window::refresh_system_status() {
        st_fps->set_value(fmt::format("{} {:.1f}", ICON_FA_FILM, ImGui::GetIO().Framerate));
        st_fps->tooltip = fmt::format(
            "Framerate: {:.1f}\nScale: {:.1f}\nSystem DPI: {}\nWindow DPI: {}",
            ImGui::GetIO().Framerate,
            get_system_scale(),
            win32::shell::get_dpi(),
            win32::shell::get_dpi((HWND)ctx.get_native_window_handle()));

        // memory usage
        win32::process p;
        uint64_t working_set;
        if(p.get_memory_info(working_set)) {
            mem_display = fmt::format("{} {}", ICON_FA_MICROCHIP, str::to_human_readable_size(working_set));
        }

        // total CPU usage
        cpu_display = fmt::format("{} {:.1f}", ICON_FA_MICROCHIP, win32::system_info::get_cpu_usage_perc());
    }
}