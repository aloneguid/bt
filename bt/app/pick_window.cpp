#include "pick_window.h"
#include "discovery.h"
#include "grey.h"
#include "../../common/str.h"
#include <string>
#include <algorithm>
#include "config.h"
#include "ui.h"
#include "../globals.h"
#include <fmt/core.h>
#include "../res.inl"
#include "win32/clipboard.h"

using namespace grey;
using namespace std;

namespace bt {
    pick_window::pick_window(
        grey::grey_context& mgr,
        url_payload up,
        float width, float height,
        std::vector<shared_ptr<browser_instance>> choices)
        : window{mgr, "Pick a Browser", width, height}, up{up}, scale{mgr.get_system_scale()} {

        for (auto bi : choices) {
            bi->popularity = config::i.get_popularity(bi->long_id());
            this->choices.push_back(bi);
        }

        sort(this->choices.begin(), this->choices.end(),
            [](const shared_ptr<browser_instance> a, const shared_ptr<browser_instance> b) -> bool {
            return a->popularity > b->popularity;
        });

        init();
    }

    void pick_window::init() {

        auto wo = make_child_window(0, 0);
        wo->padding_bottom = 70 * scale;

        auto rpt = wo->make_repeater<browser_instance>([this](repeater_bind_context<browser_instance> ctx) {

            auto& style = ImGui::GetStyle();
            //float padding = 10 * scale;
            float icon_size = 30 * scale;
            //float left_pad = icon_size + padding * 2;
            auto c = ctx.container;

            // browser logo
            auto logo = c->make_image_from_file(
                ctx.data->get_best_browser_icon_path(),
                icon_size, icon_size);
            logo->bg_draw = true;
            logo->padding_left = logo->padding_top = style.FramePadding.x;
            if(ctx.data->is_incognito) logo->alpha = 0.2;

            // browser logo
            /*c->set_pos(padding, padding, true);
            auto mimg = c->make_image_from_file(
                ctx.data->get_best_browser_icon_path(),
                icon_size, icon_size);
            if(ctx.data->is_incognito) mimg->alpha = 0.2;
            c->set_pos(0, -1);
            c->set_pos(0, -(icon_size + padding), true);*/

            // profile logo (if required)
            if(ctx.data->get_best_icon_path() != ctx.data->get_best_browser_icon_path() || ctx.data->is_incognito) {

                //c->set_pos(padding, padding + icon_size / 2, true);
                shared_ptr<image> pfi;

                if(ctx.data->is_incognito) {
                    pfi = c->make_image_from_memory("incognito",
                        incognito_icon_png, incognito_icon_png_len,
                        icon_size / 2, icon_size / 2);
                } else {
                    pfi = c->make_image_from_file(
                        ctx.data->get_best_icon_path(),
                        icon_size / 2, icon_size / 2);
                }
                pfi->bg_draw = true;
                pfi->padding_left = style.FramePadding.x;
                pfi->padding_top = style.FramePadding.y + icon_size / 2;
                pfi->rounding = icon_size / 2;
            }


            // elements
            //c->set_pos(0, -1, false);
            //c->set_pos(left_pad, padding, true);
            auto lbl_name = c->make_label(ctx.data->get_best_display_name());
            lbl_name->padding_left = icon_size + style.FramePadding.x * 2;
            lbl_name->padding_top = icon_size / 2 - 7 * scale;


            if(ctx.data->popularity > 0) {
                //c->same_line(this->width / 3 * 2);
                //c->same_line(this->width - style.FramePadding.x * 2 - style.ItemSpacing.x * 3);
                //auto wsz = ImGui::GetWindowSize();
                //ImGuiViewport* vp = ImGui::GetWindowViewport();
                //c->same_line(vp->Size.x - style.FramePadding.x * 3);
                c->same_line(this->width - style.FrameBorderSize * 4 - style.ItemSpacing.x * 8);
                auto cmd = c->make_button(fmt::format("{}", ctx.data->popularity), true, emphasis::warning);
                //cmd->is_enabled = false;
                cmd->tooltip = fmt::format("picked {}", str::humanise(ctx.data->popularity, "time", "times", "once", "twice"));
                cmd->bg_draw = true;
            }

            c->same_line();
            auto lbl_spacer = c->make_label(" ");
            lbl_spacer->padding_top = icon_size / 2;
            lbl_spacer->padding_left = -10 * scale;

            //c->set_pos(left_pad, 0, true);
            //c->make_label("#");

            /*string txt_tip = ctx.data->display_name;
            if(ctx.data->popularity > 0)
                txt_tip += fmt::format(
                    "\npicked {}",
                    str::humanise(ctx.data->popularity, "time", "times", "once", "twice"));
            ctx.container->tooltip = txt_tip;*/

        }, true);
        rpt->bind(choices);
        rpt->on_item_clicked = [this](shared_ptr<container> c, shared_ptr<browser_instance> bi) {
            up.method = "picker";
            bi->launch(up);
            bi->popularity += 1;
            config::i.set_popularity(bi->long_id(), bi->popularity);

            if(persist_rule) {
                string rule_value = txt_persist->get_value();
                str::trim(rule_value);

                bi->add_rule(rule_value);
                browser::persist_cache();
                ui::ensure_no_instances();
            }

            close();
        };


        // two buttons horizontally
        separator();
        auto g1 = make_group();
        auto cmd_cac = g1->make_button(ICON_FA_COPY " copy & cancel");
        cmd_cac->tooltip = fmt::format("copy URL to clipboard instead and cancel this dialog\nurl: {}", up.url);

        g1->same_line();
        auto cmd_persist = g1->make_button(ICON_FA_FILTER " add rule");
        cmd_persist->tooltip = "check to add this rule to configuration";

        // rule switch
        auto g2 = make_group();
        txt_persist = g2->make_input("sub");
        txt_persist->tooltip = "rule text to add";
        g2->same_line();
        auto cmd_cancel_rule = g2->make_button("cancel");
        g2->is_visible = false;

        // handlers
        cmd_persist->on_pressed = [this, g1, g2](button&) {
            persist_rule = true;
            txt_persist->set_value(str::get_domain_from_url(up.url));
            g1->is_visible = false;
            g2->is_visible = true;
        };
        cmd_cancel_rule->on_pressed = [this, g1, g2](button&) {
            persist_rule = false;
            g1->is_visible = true;
            g2->is_visible = false;
        };
        cmd_cac->on_pressed = [this](button&) {
            win32::clipboard::set_ascii_text(up.url);
            close();
        };
    }
}