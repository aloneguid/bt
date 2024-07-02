#include "picker.h"
#include <memory>
#include "../../globals.h"
#include "../../res.inl"
#include "fmt/core.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    picker_app::picker_app(const string& url, std::vector<std::shared_ptr<bt::browser_instance>> choices) 
        : url{url}, choices{choices}, title{"Pick"}, app{grey::app::make(title)}, wnd_main{ title, &is_open } {
        app->initial_theme_id = g_config.theme_id;

        // get unique list of browsers
        for(auto& c : choices) {
            if(find_if(
                browsers.begin(),
                browsers.end(), [&c](const auto& b) { return b.id == c->b->id; }) == browsers.end()) {

                browser copy = *(c->b);
                copy.instances.clear();
                browsers.push_back(copy);
            }
        }

        app->on_initialised = [this]() {

            app->preload_texture("incognito", incognito_icon_png, incognito_icon_png_len);

            // for each browser, get instances
            int max_instances{0};
            for(auto& b : browsers) {
                app->preload_texture(b.open_cmd, b.open_cmd);
                for(auto& c : this->choices) {
                    if(b.id == c->b->id) {
                        b.instances.push_back(c);
                        // pre-load icon texture once
                        if(!c->icon_path.empty()) {
                            app->preload_texture(c->icon_path, c->icon_path);
                        }
                    }
                }
                if(b.instances.size() > max_instances) {
                    max_instances = b.instances.size();
                }
            }

            auto style = ImGui::GetStyle();

            float width = (BrowserSquareSize + style.WindowPadding.x * 2 / app->scale) * browsers.size() +
                style.WindowPadding.x * 2 / app->scale;
            width = max(width, WindowMinWidth);
            float height =
                style.WindowPadding.y * 2 / app->scale +
                BrowserSquareSize +
                ProfileSquareSize +
                50;

            // calculate lef pad so browsers look centered
            browser_bar_left_pad = (width - (BrowserSquareSize * browsers.size())) / 2 * app->scale;

            wnd_main
                .size(width, height)
                .no_titlebar()
                .no_resize()
                .no_border()
                .center();

        };

    }

    picker_app::picker_app(const string& url) : picker_app::picker_app{url, browser::to_instances(browser::get_cache())} {
    }

    picker_app::~picker_app() {
    }

    void picker_app::run() {
        app->run([this](const grey::app& app) {
            return run_frame();
        });
    }

    bool picker_app::run_frame() {

        // inspiration: https://github.com/sonnyp/Junction

        w::guard gw{wnd_main};

        // URL editor
        ImGui::PushItemWidth(-1);
        w::input(url, "##url");
        ImGui::PopItemWidth();

        auto style = ImGui::GetStyle();
        w::container hz{"hz", 0, BrowserSquareSize * app->scale + style.FramePadding.y * 2};

        // horizontal container for main browser icons
        {
            w::guard g_hz{hz};

            float sq_size = BrowserSquareSize * app->scale;
            float pad = BrowserSquarePadding * app->scale;
            float full_icon_size = sq_size - pad * 2;

            int idx = 0;
            for(bt::browser& b : browsers) {

                bool dim = !b.ui_is_hovered;

                float x_start = sq_size * idx + pad + browser_bar_left_pad;
                float y_start = pad;
                w::set_pos(x_start, y_start);

                {
                    w::group g;
                    g.render();

                    if(dim) {
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, InactiveAlpha);
                        w::image(*app, b.open_cmd, full_icon_size, full_icon_size);
                        ImGui::PopStyleVar();
                    } else {
                        w::image(*app, b.open_cmd, full_icon_size, full_icon_size);
                    }

                    if(b.instances.size() > 1) {
                        w::set_pos(x_start + sq_size / 2 , y_start + sq_size / 2);
                        w::label(ICON_MD_ARROW_DROP_DOWN);
                    }
                }

                // move to bottom-right corner
                w::set_pos(sq_size * idx + pad, sq_size);

                b.ui_is_hovered = w::is_hovered();
                if(b.ui_is_hovered) {
                    active_browser_idx = idx;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
                w::tooltip(b.name);

                if(idx < (browsers.size() - 1)) {
                    w::sl();
                }

                idx++;
            }
        }

        // profiles if required
        if(active_browser_idx >= 0) {
            auto& b = browsers[active_browser_idx];
            if(b.instances.size() > 1) {

                w::container hzp{"hzp", 0, ProfileSquareSize * app->scale + style.FramePadding.y * 2};
                {
                    w::guard g_hzp{hzp};
                    auto pos = ImGui::GetCursorPos();

                    float sq_size = ProfileSquareSize * app->scale;
                    float pad = ProfileSquarePadding * app->scale;
                    float full_icon_size = sq_size - pad * 2;

                    // calculate left pad so profiles look centered
                    //float window_width = ImGui::GetWindowWidth();
                    //float browser_bar_pad = (window_width / app->scale - (ProfileSquareSize * b.instances.size())) / 2;

                    int idx{0};
                    for(auto& c : b.instances) {
                        w::set_pos(sq_size * idx + pad + browser_bar_left_pad, pad);

                        if(c->is_incognito) {
                            w::image(*app, "incognito", full_icon_size, full_icon_size);
                        } else {
                            if(c->icon_path.empty()) {
                                w::image(*app, c->b->open_cmd, full_icon_size, full_icon_size);
                            } else {
                                w::rounded_image(*app, c->icon_path, full_icon_size, full_icon_size, full_icon_size / 2);
                            }
                        }


                        w::move_pos(pad, pad);
                        if(w::is_hovered) {
                            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                        }
                        w::tooltip(c->name);

                        if(idx < (b.instances.size() - 1)) {
                            w::sl();
                        }

                        idx++;
                    }
                }
            }
        }

        return is_open;
    }
}