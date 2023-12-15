#include "url_pipeline_window.h"
#include "url_pipeline.h"
#include "../globals.h"

namespace bt {
    url_pipeline_window::url_pipeline_window(grey::grey_context& ctx) 
        : grey::window{ctx, "URL pipeline", 600, 400} {

        icon_width = 25 * scale;
        item_padding = 5 * scale;

        auto lst_pipeline_type = make_listbox("");
        lst_pipeline_type->mode = grey::listbox_mode::combo;
        lst_pipeline_type->items.emplace_back("Find/replace", "");
        lst_pipeline_type->items.emplace_back("Office 365", "");
        lst_pipeline_type->items.emplace_back("Un-shorten", "");
        lst_pipeline_type->selected_index = 0;
        lst_pipeline_type->width = 150 * scale;

        same_line();
        auto cmd_add = make_button(ICON_FA_CIRCLE_PLUS " add");
        cmd_add->set_emphasis(grey::emphasis::primary);
        cmd_add->tooltip = "Add a new step to the pipeline.";
        same_line();

        auto cmd_reset = make_button(ICON_FA_TRASH " reset");
        cmd_reset->set_emphasis(grey::emphasis::error);
        cmd_reset->tooltip = "Reset to default pipeline.";
        separator();

        // add repeater into a sub-window, this will allow separate scrolling area
        auto rpt_container = make_child_window();

        auto rpt = rpt_container->make_repeater<url_pipeline_step>(
            [this](auto bctx) {make_ui_step(bctx); },
            true);
        rpt->bind(g_pipeline.get_steps());
    }

    void url_pipeline_window::make_ui_step(grey::repeater_bind_context<url_pipeline_step> ctx) {
        switch(ctx.data->type) {
            case url_pipeline_step_type::unshortener:
                make_unconfigurable_step(ctx.container, ICON_FA_ARROW_UP_WIDE_SHORT, "Un-shorten");
                break;
            case url_pipeline_step_type::o365:
                make_unconfigurable_step(ctx.container, ICON_FA_MICROSOFT, "Unwrap Office 365");
                break;
            case url_pipeline_step_type::replacer:
                make_unconfigurable_step(ctx.container, ICON_FA_MAGNIFYING_GLASS_ARROW_RIGHT, "Find/replace");
                make_replacer_step(ctx.container, ctx.data);
                break;
            default:
                make_unconfigurable_step(ctx.container, ICON_FA_QUESTION, "Unknown");
                break;
        }
    }

    void url_pipeline_window::make_unconfigurable_step(std::shared_ptr<grey::container> container,
        const std::string& icon,
        const std::string& title) {

        auto cmd_up = container->make_button(ICON_FA_SORT_UP, true);
        cmd_up->padding_left;
        container->same_line();
        auto cmd_down = container->make_button(ICON_FA_SORT_DOWN, true);

        container->same_line();
        auto txt_icon = container->make_label(icon);
        txt_icon->set_emphasis(grey::emphasis::primary);
        txt_icon->set_padding(item_padding);

        container->same_line(icon_width);
        auto txt_title = container->make_label(title);
        txt_title->set_padding(0, item_padding, 0, item_padding);
    }

    void url_pipeline_window::make_replacer_step(std::shared_ptr<grey::container> container,
        std::shared_ptr<bt::url_pipeline_step> step) {

        container->make_label(""); container->make_label("");
        container->same_line(icon_width);
        auto type = container->make_listbox("");
        type->width = 90 * scale;
        type->mode = grey::listbox_mode::combo;
        type->items.emplace_back("Substring", "");
        type->items.emplace_back("Regex", "");
        type->selected_index = 0;

        float left_pad = icon_width + 100 * scale;

        container->same_line(left_pad);
        auto txt_find = container->make_input("find");

        container->make_label("");
        container->same_line(left_pad);
        auto txt_replace = container->make_input("replace");
        txt_replace->set_padding(0, 0, 0, item_padding);
    }
}