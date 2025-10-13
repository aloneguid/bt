#include "btwidgets.h"
#include "../../res.inl"
#include "../../globals.h"
#include "fss.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    void btw_on_app_initialised(grey::app& app) {
        app.preload_texture("incognito", incognito_icon_png, incognito_icon_png_len);

        // preload browser icons
        for(auto& b : g_config.browsers) {
            string path = b->get_best_icon_path();
            app.preload_texture(path, fss::get_full_path(path));

            // preload browser instances
            for(auto& bi : b->instances) {
                string path = bi->get_best_icon_path();
                app.preload_texture(path, fss::get_full_path(path));
            }
        }
    }

    void btw_icon(grey::app& app, std::shared_ptr<bt::browser_instance> bi, float padding, float icon_size, bool is_active) {

        float box_size = icon_size + padding * 2;

        w::group g;
        g.render();

        // render icon and come back to starting position
        float x0, y0;
        w::cur_get(x0, y0);

        // dummy
        ImGui::Dummy(ImVec2{box_size, box_size});

        w::cur_set(x0 + padding, y0 + padding);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, is_active ? 1 : g_config.picker_inactive_item_alpha);

        w::rounded_image(app, bi->b->get_best_icon_path(), icon_size, icon_size, icon_size / 2);

        // if required, draw profile icon
        if(!bi->is_singular()) {
            w::cur_set(x0 + padding + icon_size / 2, y0 + padding + icon_size / 2);
            float isz = icon_size / 2;
            if(bi->is_incognito && bi->user_icon_path.empty()) {
                w::image(app, "incognito", isz, isz);
            } else {
                w::rounded_image(app, bi->get_best_icon_path(), isz, isz, isz);
            }
        }

        ImGui::PopStyleVar();
    }
}