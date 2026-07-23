#pragma once
#include "grey.h"
#include "../strings.h"

namespace bt::ui {

    namespace w = grey::widgets;

    class ww {
    public:
        static void help_link(const std::string& relative_url) {
            w::label(ICON_MD_LIGHTBULB, 0, false);
            if(w::is_hovered()) {
                w::tt(bt::strings::OnlineHelpTooltip);
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
            if(w::is_leftclicked()) {
                auto url = APP_HELP_BASE_URL + relative_url;
                url_opener::open(url);
            }
        }
    };
}