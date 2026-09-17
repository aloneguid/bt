#include "viewer_app.h"
#include "../../globals.h"
#include "btwidgets.h"

using namespace std;
namespace w = grey::widgets;
using namespace grey;

namespace bt::ui {
    viewer_app::viewer_app(const string& url) : url{url}, app{app::make("viewer", sz{600, 200})} {
        app->fonts.load_all();
        app->initial_theme_id = g_state.ui_theme;
        app->can_resize = true;
        app->chrome = system_chrome::standard;

        auto& opts = app->main_window_opts();
        opts.open_ptr = &is_open;
        opts.scrollable = true;
        opts.title = "URL Viewer";

        app->on_initialised = [this]() {
            btw_on_app_initialised(*app);
        };
    }

    void viewer_app::run() {
        app->run([this]() {
            w::lbl("Opened URL:");
            w::sl();
            if (w::btn("Copy")) {
                ImGui::SetClipboardText(url.c_str());
            }

            ImGui::InputTextMultiline("##url", (char*)url.c_str(), url.size(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4), ImGuiInputTextFlags_ReadOnly);

            if (w::btn("Close")) {
                is_open = false;
            }

            return is_open;
        });
    }
}
