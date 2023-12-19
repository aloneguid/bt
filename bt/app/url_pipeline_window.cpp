#include "url_pipeline_window.h"
#include "url_pipeline.h"
#include "../globals.h"
#include "pipeline/replacer.h"

using namespace std;

namespace bt {
    url_pipeline_window::url_pipeline_window(grey::grey_context& ctx) 
        : grey::window{ctx, "URL pipeline", 650, 400} {

        icon_width = 25 * scale;
        item_padding = 5 * scale;

        auto lst_pipeline_type = make_listbox("", 0);
        lst_pipeline_type->mode = grey::listbox_mode::combo;
        lst_pipeline_type->items.emplace_back(url_pipeline_step::to_string(url_pipeline_step_type::find_replace), "");
        lst_pipeline_type->items.emplace_back(url_pipeline_step::to_string(url_pipeline_step_type::o365), "");
        lst_pipeline_type->items.emplace_back(url_pipeline_step::to_string(url_pipeline_step_type::unshortener), "");
        lst_pipeline_type->width = 150 * scale;

        same_line();
        auto cmd_add = make_button(ICON_FA_CIRCLE_PLUS " add");
        cmd_add->set_emphasis(grey::emphasis::primary);
        cmd_add->tooltip = "Add a new step to the pipeline.";
        cmd_add->on_pressed = [this, lst_pipeline_type](grey::button&) {
            auto type = static_cast<url_pipeline_step_type>(lst_pipeline_type->get_selected_index() + 1);
            add_step(type);
        };

        same_line();
        auto cmd_reset = make_button(ICON_FA_TRASH " reset");
        cmd_reset->set_emphasis(grey::emphasis::error);
        cmd_reset->tooltip = "Reset to default pipeline.";

        // step buttons
        same_line(); make_label("|")->is_enabled = false;
        same_line();
        auto cmd_up = make_button(ICON_FA_ARROW_UP, true);
        cmd_up->tooltip = "Move selected step up.";
        cmd_up->is_enabled = false;
        same_line();
        auto cmd_down = make_button(ICON_FA_ARROW_DOWN, true);
        cmd_down->tooltip = "Move selected step down.";
        cmd_down->is_enabled = false;
        same_line();
        auto cmd_delete = make_button(ICON_FA_TRASH, " delete");
        cmd_delete->tooltip = "Delete selected step.";
        cmd_delete->is_enabled = false;

        separator();

        // add repeater into a sub-window, this will allow separate scrolling area
        auto rpt_container = make_child_window();

        rpt = rpt_container->make_repeater<url_pipeline_step>(
            [this](auto bctx) {make_ui_step(bctx); },
            true);
        rpt->bind(g_pipeline.get_steps());
    }

    void url_pipeline_window::make_ui_step(grey::repeater_bind_context<url_pipeline_step> ctx) {
        switch(ctx.data->type) {
            case url_pipeline_step_type::unshortener:
                make_unconfigurable_step(ctx.container, to_icon(ctx.data->type), url_pipeline_step::to_string(ctx.data->type));
                break;
            case url_pipeline_step_type::o365:
                make_unconfigurable_step(ctx.container, to_icon(ctx.data->type), url_pipeline_step::to_string(ctx.data->type));
                break;
            case url_pipeline_step_type::find_replace:
                make_unconfigurable_step(ctx.container, to_icon(ctx.data->type), url_pipeline_step::to_string(ctx.data->type));
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

        auto txt_icon = container->make_label(icon);
        txt_icon->set_emphasis(grey::emphasis::primary);
        txt_icon->set_padding(item_padding);

        container->same_line(icon_width);
        auto txt_title = container->make_label(title);
        txt_title->set_padding(0, item_padding, 0, item_padding);
    }

    void url_pipeline_window::make_replacer_step(std::shared_ptr<grey::container> container,
        std::shared_ptr<bt::url_pipeline_step> step) {

        std::shared_ptr<bt::pipeline::replacer> replacer_step = std::static_pointer_cast<bt::pipeline::replacer>(step);

        container->make_label(""); container->make_label("");
        container->same_line(icon_width);
        auto type = container->make_listbox("", 0);
        type->width = 90 * scale;
        type->mode = grey::listbox_mode::combo;
        type->items.emplace_back("Substring", "");
        type->items.emplace_back("Regex", "");
        type->on_selected = [replacer_step](size_t idx, grey::list_item& li) {
            replacer_step->kind = static_cast<bt::pipeline::replacer_kind>(idx);
        };

        float left_pad = icon_width + 100 * scale;

        container->same_line(left_pad);
        auto txt_find = container->make_input("find", &replacer_step->find);

        container->make_label("");
        container->same_line(left_pad);
        auto txt_replace = container->make_input("replace", &replacer_step->replace);
        txt_replace->set_padding(0, 0, 0, item_padding);
    }

    void url_pipeline_window::add_step(url_pipeline_step_type type) {
        switch(type) {
            case url_pipeline_step_type::find_replace:
                g_pipeline.get_steps().emplace_back(std::make_shared<bt::pipeline::replacer>(""));
                break;
            case url_pipeline_step_type::o365:
                break;
            case url_pipeline_step_type::unshortener:
                break;
            default:
                break;
        }

        // re-bind repeater
        rpt->bind(g_pipeline.get_steps());
    }

    std::string url_pipeline_window::to_icon(url_pipeline_step_type type) {
        switch(type) {
            case url_pipeline_step_type::find_replace: return ICON_FA_MAGNIFYING_GLASS_ARROW_RIGHT;
            case url_pipeline_step_type::o365: return ICON_FA_MICROSOFT;
            case url_pipeline_step_type::unshortener: return ICON_FA_ARROW_UP_WIDE_SHORT;
            default: return ICON_FA_QUESTION;
        }
    }
}