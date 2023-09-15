#pragma once
#include "grey.h"
#include <string>
#include "browser.h"

namespace bt {
    class pick_window : public grey::window {
    public:
        pick_window(
            grey::grey_context& mgr,
            url_payload up,
            float width, float height,
            std::vector<std::shared_ptr<browser_instance>> choices);

        void init();

    private:
        const float SQUARE_SIZE = 200;
        const float SQUARE_PADDING = 8;
        const float ICON_SIZE = 100;
        const float ICON_LEVITATION = 10;

        url_payload up;
        std::vector<std::shared_ptr<browser_instance>> choices;
        bool persist_domain{false};
        bool persist_popularity{false};

        void launch(std::shared_ptr<browser_instance> bi);
    };
}