#pragma once
#include "grey.h"
#include "url_pipeline_step.h"

namespace bt {
    /**
     * @brief Configuration UI for the URL pipeline
    */
    class url_pipeline_window : public grey::window {
    public:
        url_pipeline_window(grey::grey_context& ctx);

    private:

        float icon_width;
        float item_padding;
        std::shared_ptr<grey::repeater<url_pipeline_step>> rpt;
        std::shared_ptr<grey::button> cmd_up;
        std::shared_ptr<grey::button> cmd_down;
        std::shared_ptr<grey::button> cmd_delete;

        /**
         * @brief Convert to FA_ icon.
         */
        static std::string to_icon(url_pipeline_step_type type);

        void make_ui_step(grey::repeater_bind_context<url_pipeline_step> ctx);
        void make_unconfigurable_step(std::shared_ptr<grey::container> container,
            const std::string& icon,
            const std::string& title);
        void make_replacer_step(std::shared_ptr<grey::container> container, std::shared_ptr<bt::url_pipeline_step> step);

        void add_step(url_pipeline_step_type type);

        void move_active_step(int direction);
        void delete_active_step();

        void update_step_manipulation_buttons();
    };
}