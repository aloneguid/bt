#include "picker.h"
#include <memory>
#include "../../globals.h"
#include "../../res.inl"
#include "fmt/core.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    picker_app::picker_app(std::vector<std::shared_ptr<bt::browser_instance>> choices) 
        : choices{choices}, title{"Pick"}, app{grey::app::make(title)}, wnd_main{ title, &is_open } {
        app->initial_theme_id = g_config.theme_id;

        wnd_main
            .size(400, 300)
            .center();

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

        // for each browser, get instances
        for(auto& b : browsers) {
            for(auto& c : choices) {
                if(b.id == c->b->id) {
                    b.instances.push_back(c);
                }
            }
        }
    }

    picker_app::picker_app() : picker_app::picker_app{browser::to_instances(browser::get_cache())} {
    }

    picker_app::~picker_app() {
    }

    void picker_app::run() {
        app->run([this](const grey::app& app) {
            return run_frame();
        });
    }

    bool picker_app::run_frame() {

        w::guard gw{wnd_main};
        w::container hz{"hz"};
        //hz.border();

        // horizontal container
        {
            w::guard g_hz{hz};

            float b_is = BrowserIconSize * app->scale;
            float p_is = ProfileIconSize * app->scale;

            for(auto& b : browsers) {

                app->preload_texture(b.open_cmd, b.open_cmd);

                // vertical container per browser
                w::container vert{b.id, (BrowserIconSize + 10) * app->scale};
                vert.border();
                {
                    w::guard g_vert{vert};

                    w::image(*app, b.open_cmd, b_is, b_is);

                    for(auto& c : b.instances) {
                        //w::label(c->name);

                        if(c->is_incognito) {
                            app->preload_texture("incognito", incognito_icon_png, incognito_icon_png_len);
                            w::image(*app, "incognito", p_is, p_is);
                        } else {
                            if(c->icon_path.empty()) {
                                w::image(*app, b.open_cmd, p_is, p_is);
                            } else {
                                app->preload_texture(c->icon_path, c->icon_path);
                                w::rounded_image(*app, c->icon_path, p_is, p_is, p_is / 2);
                                //w::move_pos(0, p_is);
                            }
                        }

                        w::tooltip(fmt::format("{}", c->name));
                        
                    }
                }

                w::sl();
            }
        }

        return is_open;
    }
}