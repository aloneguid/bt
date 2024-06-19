#include "url_pipeline_window.h"
#include "url_pipeline.h"
#include "../globals.h"
#include "stl.hpp"
#include "pipeline/replacer.h"
#include "pipeline/o365.h"
#include "pipeline/unshortener.h"

using namespace std;

namespace bt {
    url_pipeline_window::url_pipeline_window(grey::grey_context& ctx) 
        : grey::window{ctx, "URL pipeline", 650, 400} {

        icon_width = 25 * scale;
        item_padding = 5 * scale;

        // step type combo
        auto lst_step_type = make_listbox("", 0);
        lst_step_type->mode = grey::listbox_mode::combo;
        lst_step_type->items.emplace_back(url_pipeline_step::to_string(url_pipeline_step_type::find_replace), "");
        lst_step_type->items.emplace_back(url_pipeline_step::to_string(url_pipeline_step_type::o365), "");
        lst_step_type->items.emplace_back(url_pipeline_step::to_string(url_pipeline_step_type::unshortener), "");
        lst_step_type->width = 150 * scale;

        // "add step to pipeline" button
        same_line();
        auto cmd_add = make_button(ICON_FA_CIRCLE_PLUS " add");
        cmd_add->set_emphasis(grey::emphasis::primary);
        cmd_add->tooltip = "Add a new step (from drop-down on the left) to the pipeline.";
        cmd_add->on_pressed = [this, lst_step_type](grey::button&) {
            auto type = static_cast<url_pipeline_step_type>(lst_step_type->get_selected_index() + 1);
            add_step(type);
        };

        same_line();
        auto cmd_reset = make_button(ICON_FA_TRASH " reset");
        cmd_reset->set_emphasis(grey::emphasis::error);
        cmd_reset->tooltip = "Reset to default pipeline.";

        // step buttons
        same_line(); make_label("|")->is_enabled = false;
        same_line();
        cmd_up = make_button(ICON_FA_ARROW_UP, true);
        cmd_up->tooltip = "Move selected step up.";
        cmd_up->is_enabled = false;
        cmd_up->on_pressed = [this](grey::button&) {
            move_active_step(-1);
        };
        same_line();
        cmd_down = make_button(ICON_FA_ARROW_DOWN, true);
        cmd_down->tooltip = "Move selected step down.";
        cmd_down->is_enabled = false;
        cmd_down->on_pressed = [this](grey::button&) {
            move_active_step(1);
        };
        same_line();
        cmd_delete = make_button(ICON_FA_TRASH, " delete");
        cmd_delete->tooltip = "Delete selected step.";
        cmd_delete->is_enabled = false;
        cmd_delete->on_pressed = [this](grey::button&) {
            delete_active_step();
        };

        separator();

        // add repeater into a sub-window, this will allow separate scrolling area
        auto rpt_container = make_child_window();

        rpt = rpt_container->make_repeater<url_pipeline_step>(
            [this](auto bctx) {
                make_ui_step(bctx);
            },
            true);
        rpt->bind(g_pipeline.get_steps());
        rpt->on_item_clicked = [this](
            size_t idx, shared_ptr<grey::container>, shared_ptr<url_pipeline_step>) {
            update_step_manipulation_buttons();
        };

        cmd_reset->on_pressed = [this](grey::button&) {
            g_pipeline.reset();
            rpt->bind(g_pipeline.get_steps());
        };
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
        type->set_selected_index(static_cast<int>(replacer_step->kind));
        type->on_selected = [replacer_step](size_t idx, grey::list_item& li) {
            replacer_step->kind = static_cast<bt::pipeline::replacer_kind>(idx);
            g_pipeline.save();
        };

        float left_pad = icon_width + 100 * scale;

        container->same_line(left_pad);
        auto txt_find = container->make_input("find", &replacer_step->find);
        txt_find->on_value_changed = [replacer_step](const std::string& value) {
            g_pipeline.save();
        };

        container->make_label("");
        container->same_line(left_pad);
        auto txt_replace = container->make_input("replace", &replacer_step->replace);
        txt_replace->set_padding(0, 0, 0, item_padding);
        txt_replace->on_value_changed = [replacer_step](const std::string& value) {
            g_pipeline.save();
        };
    }

    void url_pipeline_window::add_step(url_pipeline_step_type type) {
        switch(type) {
            case url_pipeline_step_type::find_replace:
                g_pipeline.get_steps().emplace_back(std::make_shared<bt::pipeline::replacer>(""));
                break;
            case url_pipeline_step_type::o365:
                g_pipeline.get_steps().emplace_back(std::make_shared<bt::pipeline::o365>());
                break;
            case url_pipeline_step_type::unshortener:
                g_pipeline.get_steps().emplace_back(std::make_shared<bt::pipeline::unshortener>());
                break;
            default:
                break;
        }
        g_pipeline.save();

        // re-bind repeater
        rpt->bind(g_pipeline.get_steps());
    }

    void url_pipeline_window::move_active_step(int direction) {
        int idx = rpt->get_selected_index();
        stl::move(g_pipeline.get_steps(), rpt->get_selected_index(), direction, true);
        rpt->bind(g_pipeline.get_steps());
        rpt->set_selected_index(idx + direction);
        update_step_manipulation_buttons();
        g_pipeline.save();
    }

    void url_pipeline_window::delete_active_step() {
        if(g_pipeline.get_steps().size() < 2) return;
        int idx = rpt->get_selected_index();

        g_pipeline.get_steps().erase(g_pipeline.get_steps().begin() + rpt->get_selected_index());
        rpt->bind(g_pipeline.get_steps());
        int count = g_pipeline.get_steps().size();
        rpt->set_selected_index(idx < count ? idx : count - 1);
        update_step_manipulation_buttons();
        g_pipeline.save();
    }

    void url_pipeline_window::update_step_manipulation_buttons() {
        int idx = rpt->get_selected_index();
        cmd_up->is_enabled = idx;
        cmd_down->is_enabled = idx < g_pipeline.get_steps().size() - 1;
        cmd_delete->is_enabled = g_pipeline.get_steps().size() > 1; // avoid deleting last step
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