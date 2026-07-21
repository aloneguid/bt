#include "btwidgets.h"
#include "../../globals.h"
#include "../../res.h"
#include "fss.h"

using namespace std;
using namespace grey::common;
namespace w = grey::widgets;

namespace bt::ui {
    void btw_on_app_initialised(grey::app& app) {
        app.preload_texture("incognito", incognito_icon_png, incognito_icon_png_len);

        // preload browser icons
        for(auto& b : g_state.browsers) {
            string browser_path = b.get_best_icon_path();
            app.preload_texture(browser_path, fss::get_full_path(browser_path));

            // preload browser profiles
            for(const browser_profile& profile : b.profiles) {
                string profile_path = b.get_best_icon_path(profile);
                app.preload_texture(profile_path, fss::get_full_path(profile_path));
            }
        }
    }

    void btw_icon(grey::app& app,
        const profile_selection& selection,
        float padding, float icon_size, bool is_active) {
        btw_icon(app, selection.b(), selection.profile(), g_state.icon_overlay, padding, icon_size, is_active);

    }

    void btw_icon(grey::app &app, const browser &b, const browser_profile &p,
        icon_overlay_mode icon_mode,
        float padding, float icon_size,
        bool is_active) {

        float box_size = icon_size + padding * 2;

        w::group g;

        // render icon and come back to starting position
        float x0, y0;
        w::cur_get(x0, y0);

        // dummy
        ImGui::Dummy(ImVec2{box_size, box_size});

        w::cur_set(x0 + padding, y0 + padding);

        string icon1 = b.get_best_icon_path();
        string icon2;
        if(b.engine != browser_engine::generic) {
            if(p.is_incognito && p.user_icon_path.empty())
                icon2 = "incognito";
            else
                icon2 = b.get_best_icon_path(p);
        }

        switch(icon_mode) {
            case icon_overlay_mode::browser_only:
                icon2 = "";
                break;
            case icon_overlay_mode::profile_only:
                icon1 = icon2.empty() ? icon1 : icon2;
                icon2.clear();
                break;
            case icon_overlay_mode::browser_on_profile:
                if(!icon2.empty())
                    std::swap(icon1, icon2);
                break;
        }
        if(icon2 == icon1)
            icon2.clear();

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, is_active ? 1 : g_state.picker.inactive_item_alpha);

        w::image_rounded(app, icon1, icon_size, icon_size, icon_size / 2);

        // if required, draw overlay icon
        if(!icon2.empty()) {
            w::cur_set(x0 + padding + icon_size / 2, y0 + padding + icon_size / 2);
            float isz = icon_size / 2;
            w::image_rounded(app, icon2, isz, isz, isz);
        }

        ImGui::PopStyleVar();

        if(p.use_color || p.use_user_color) {
            // user color has priority
            unsigned color = p.use_user_color ? p.user_color : p.color;

            //draw circle around the icon with user color
            auto dl = ImGui::GetWindowDrawList();
            ImVec2 center{x0 + padding + icon_size / 2, y0 + padding + icon_size / 2};
            auto radius = icon_size / 2 + g_state.highlight_width * w::scale;
            dl->AddCircle(center, radius, color, 0, g_state.highlight_width * w::scale);
        }
    }
}
