#include "picker_app.h"
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

            wnd_width = (BrowserSquareSize + style.WindowPadding.x * 2 / app->scale) * browsers.size() +
                style.WindowPadding.x * 2 / app->scale;
            wnd_width = max(wnd_width, WindowMinWidth);
            wnd_height_normal =
                style.WindowPadding.y * 2 / app->scale +
                BrowserSquareSize +
                //ProfileSquareSize +
                25;
            wnd_height_profiles = wnd_height_normal + ProfileSquareSize;

            // calculate lef pad so browsers look centered
            browser_bar_left_pad = (wnd_width - (BrowserSquareSize * browsers.size())) / 2 * app->scale;

            wnd_main
                .size(wnd_width, wnd_height_normal)
                .no_titlebar()
                .no_resize()
                //.no_border()
                .no_scroll()
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

        if(w::button(ICON_MD_CLOSE, w::emphasis::none, true, true)) {
            wnd_main.resize(100, 100);
        }
        w::sl();
        w::button(ICON_MD_SETTINGS, w::emphasis::none, true, true);

        // URL editor
        ImGui::PushItemWidth(-1);
        w::sl();
        w::input(url, "##url");
        ImGui::PopItemWidth();
        w::tooltip("You can change this URL before making a decision to change which URL will be invoked");

        render_browser_bar();
        render_profile_bar();
        render_connection_box();

        return is_open;
    }

    void picker_app::render_browser_bar() {
        float sq_size = BrowserSquareSize * app->scale;
        float pad = BrowserSquarePadding * app->scale;
        float pad1 = InactiveBrowserSquarePadding * app->scale;
        float full_icon_size = sq_size - pad * 2;
        float full_icon_size1 = sq_size - pad1 * 2;
        float x, y;
        w::get_pos(x, y);

        int idx = 0;
        for(bt::browser& b : browsers) {

            float box_x_start = sq_size * idx + browser_bar_left_pad;
            float box_y_start = y;

            w::set_pos(box_x_start, box_y_start);
            bool is_multiple_choice = b.instances.size() > 1;

            {
                w::group g;
                g.render();

                ImGui::Dummy(ImVec2(sq_size, sq_size));
                w::set_pos(box_x_start, box_y_start);

                if(b.ui_is_hovered || idx == active_browser_idx) {
                    w::move_pos(pad, pad);
                    w::image(*app, b.open_cmd, full_icon_size, full_icon_size);
                } else {
                    float pad1 = InactiveBrowserSquarePadding * app->scale;
                    float diff = pad1 - pad;
                    float isz = sq_size - pad1 * 2;

                    w::move_pos(pad1, pad1);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, InactiveAlpha);
                    w::image(*app, b.open_cmd, full_icon_size1, full_icon_size1);
                    ImGui::PopStyleVar();
                }
            }

            // move to bottom-right corner
            w::set_pos(sq_size * idx + pad, sq_size);

            b.ui_is_hovered = w::is_hovered();
            if(b.ui_is_hovered) {
                if(active_browser_idx != idx) {
                    active_profile_idx = -1;
                    profiles_cb.clear();
                }

                active_browser_idx = idx;
                if(!is_multiple_choice) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    if(w::is_leftclicked()) {
                        make_decision(b.instances[0]);
                    }
                }
            }
            w::tooltip(b.name);

            if(active_browser_idx == idx) {
                // this needs to be re-calculatd on each frame
                active_browser_cb.min = ImGui::GetItemRectMin();
                active_browser_cb.max = ImGui::GetItemRectMax();
            }

            if(idx < (browsers.size() - 1)) {
                w::sl();
            }

            idx++;
        }

        w::set_pos(0, y + BrowserSquareSize * app->scale);
    }

    void picker_app::render_profile_bar() {

        bool have_bar{false};

        // profiles if required
        if(active_browser_idx >= 0) {
            auto& b = browsers[active_browser_idx];
            if(b.instances.size() > 1) {
                have_bar = true;
                w::move_pos(0, ProfileSquarePadding * app->scale);  // some distance

                float sq_size = ProfileSquareSize * app->scale;
                float pad = ProfileSquarePadding * app->scale;
                float pad1 = InactiveProfileSquarePadding * app->scale;
                float full_icon_size = sq_size - pad * 2;
                float full_icon_size1 = sq_size - pad1 * 2;
                float x, y;
                w::get_pos(x, y);

                {
                    // calculate left pad so profiles look centered
                    //float window_width = ImGui::GetWindowWidth();
                    //float browser_bar_pad = (window_width / app->scale - (ProfileSquareSize * b.instances.size())) / 2;

                    int idx{0};
                    if(profiles_cb.size() != b.instances.size()) {
                        profiles_cb.resize(b.instances.size());
                    }
                    for(auto& c : b.instances) {
                        float box_x_start = sq_size * idx + browser_bar_left_pad;
                        float box_y_start = y;

                        w::set_pos(box_x_start, box_y_start);

                        {
                            w::group g;
                            g.render();

                            ImGui::Dummy(ImVec2(sq_size, sq_size));
                            w::set_pos(box_x_start, box_y_start);

                            bool big = c->ui_is_hovered || idx == active_profile_idx;
                            float isz = big ? full_icon_size : full_icon_size1;
                            float mpd = big ? pad : pad1;
                            w::move_pos(mpd, mpd);

                            if(c->is_incognito) {
                                w::image(*app, "incognito", isz, isz);
                            } else {
                                if(c->icon_path.empty()) {
                                    w::image(*app, c->b->open_cmd, isz, isz);
                                } else {
                                    w::rounded_image(*app, c->icon_path, isz, isz, isz / 2);
                                }
                            }
                        }

                        c->ui_is_hovered = w::is_hovered();

                        if(c->ui_is_hovered) {
                            active_profile_idx = idx;
                            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                        }
                        w::tooltip(c->name);
                        if(w::is_leftclicked()) {
                            make_decision(c);
                        }

                        profiles_cb[idx] = {ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};

                        if(idx < (b.instances.size() - 1)) {
                            w::sl();
                        }

                        idx++;
                    }
                }
            }
        }
    
        // resize window to fit profiles, or cut it back to normal
        if(have_bar == wnd_height_is_normal) {
            wnd_height_is_normal = !have_bar;
            wnd_main.resize(wnd_width, wnd_height_is_normal ? wnd_height_normal : wnd_height_profiles);
        }
    }

    void picker_app::render_connection_box() {
        if(active_browser_cb.min.x == 0) return;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        //ImDrawList* dl = ImGui::GetBackgroundDrawList();

        ImU32 col_box = w::imcol32(ImGuiCol_Border);
        ImU32 col_dot = w::imcol32(ImGuiCol_Text);

        //dl->AddRect(active_browser_cb.min, active_browser_cb.max, col_box, 5, 0);

        // calculate dots
        ImVec2 p0{
            (active_browser_cb.max.x - BrowserSquareSize * app->scale / 2),
            active_browser_cb.max.y - (InactiveBrowserSquarePadding / 2) * app->scale};

        dl->AddCircleFilled(p0, 2, col_dot);

        for(int i = 0; i < profiles_cb.size(); i++) {
            auto& cb = profiles_cb[i];
            ImVec2 p4{
                (cb.min.x + cb.max.x) / 2,
                cb.min.y + (InactiveProfileSquarePadding / 2) * app->scale};
            dl->AddCircleFilled(p4, 2, col_dot);

            // draw connecting bezier curve
            ImVec2 p1{p0.x, p0.y + BrowserSquarePadding * app->scale};
            ImVec2 p2{p4.x, p4.y - ProfileSquarePadding * app->scale};

            // comment out to see orientation dots
            //dl->AddCircleFilled(p1, 2, col_dot);
            //dl->AddCircleFilled(p2, 2, col_dot);

            float thickness = i == active_profile_idx ? 3 : 1;

            dl->AddBezierCubic(p0, p1, p2, p4, col_dot, thickness, 32);
        }

        //ImVec2 p0{min.x + 50, min.y + 50};
        //ImVec2 p1{min.x + 50, min.y + 100};
        //ImVec2 p2{min.x + 100, min.y + 250};
        //ImVec2 p3{min.x + 100, min.y + 300};

        //dl->AddCircleFilled(p0, 5, IM_COL32(255, 0, 0, 255), 32);
        //dl->AddCircleFilled(p1, 5, IM_COL32(255, 0, 0, 255), 32);
        //dl->AddCircleFilled(p2, 5, IM_COL32(255, 0, 0, 255), 32);
        //dl->AddCircleFilled(p3, 5, IM_COL32(255, 0, 0, 255), 32);

        //dl->AddBezierCubic(p0, p1, p2, p3, IM_COL32(0, 255, 0, 255), 2, 32);
    }

    void picker_app::make_decision(std::shared_ptr<bt::browser_instance> decision) {
        is_open = false;
    }
}