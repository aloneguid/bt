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
        : window{mgr, "Pick a Browser", width, height}, up{up} {

        persist_popularity = g_config.get_persist_popularity();

        for (const auto bi : choices) {
            bi->popularity = g_config.get_popularity(bi->long_id());
            this->choices.push_back(bi);
        }

        sort(this->choices.begin(), this->choices.end(),
            [](const shared_ptr<browser_instance> a, const shared_ptr<browser_instance> b) -> bool {
            return a->popularity > b->popularity;
        });

        init();
    }

    void pick_window::init() {

        same_line(15 * scale);
        auto cmd_cac = make_button(ICON_FA_COPY);
        cmd_cac->tooltip = fmt::format("Copy URL to clipboard and cancel picker without opening any browser\nurl: {}", up.url);

        same_line();
        auto chk_app_mode = make_checkbox(ICON_FA_CROP, &up.app_mode);
        chk_app_mode->render_as_icon = true;
        chk_app_mode->tooltip = "Try to open in frameless window.";

        same_line();
        auto chk_persist_domain = make_checkbox(ICON_FA_CIRCLE_PLUS, &persist_domain);
        chk_persist_domain->render_as_icon = true;
        chk_persist_domain->tooltip = fmt::format(
            "Add domain of this url ({}) as a rule to a browser you click.", str::get_domain_from_url(up.url));

        same_line();
        make_label("|")->is_enabled = false;

        same_line();
        auto chk_persist_popularity = make_checkbox(ICON_FA_ARROW_DOWN_9_1, &persist_popularity);
        chk_persist_popularity->render_as_icon = true;
        chk_persist_popularity->tooltip = "Record number of clicks to sort in descending order by this number.";
        chk_persist_popularity->on_value_changed = [](bool v) { g_config.set_persist_popularity(v); };

        // process name (if present)
        if(!up.process_name.empty()) {
            same_line();
            auto lbl_pn = make_label(ICON_FA_MICROCHIP);
            lbl_pn->is_enabled = false;
            lbl_pn->tooltip = fmt::format("process name: {}", up.process_name);
        }

        // window title (if present)
        if(!up.window_title.empty()) {
            same_line();
            auto lbl = make_label(ICON_FA_WINDOW_MAXIMIZE);
            lbl->is_enabled = false;
            lbl->tooltip = fmt::format("window title:\n{}", up.window_title);
        }

        auto wo = make_child_window(0, 0);
        //wo->padding_bottom = 89 * scale;

        auto rpt = wo->make_repeater<browser_instance>(
            [this](repeater_bind_context<browser_instance> ctx) {

            auto& style = ImGui::GetStyle();
            float icon_size = 30 * scale;
            auto c = ctx.container;

            // keyboard accelerator
            const float acc_width = 18 * scale;
            if(ctx.idx < 9) {
                auto lbl_acc = c->make_label(fmt::format("{}", ctx.idx + 1));
                lbl_acc->bg_draw = true;
                lbl_acc->padding_top = 11 * scale;
                lbl_acc->padding_left = 2 * scale;
                lbl_acc->set_emphasis(emphasis::error);
                lbl_acc->tooltip = fmt::format("press {} to make selection with keyboard", ctx.idx + 1);
            }

            string tooltip = fmt::format("browser: {}\nprofile: {}\npicked: {} time(s)",
                ctx.data->b->name,
                ctx.data->name,
                ctx.data->popularity);

            // browser logo
            auto logo = c->make_image_from_file(
                ctx.data->b->open_cmd,
                icon_size, icon_size);
            logo->bg_draw = true;
            logo->padding_left = logo->padding_top = style.FramePadding.x;
            logo->padding_left += acc_width;
            logo->tooltip = tooltip;
            if(ctx.data->is_incognito) logo->alpha = 0.3;

            // profile logo (if required)
            if(!ctx.data->icon_path.empty() || ctx.data->is_incognito) {

                //c->set_pos(padding, padding + icon_size / 2, true);
                shared_ptr<image> pfi;

                if(ctx.data->is_incognito) {
                    pfi = c->make_image_from_memory("incognito",
                        incognito_icon_png, incognito_icon_png_len,
                        icon_size / 2, icon_size / 2);
                } else {
                    pfi = c->make_image_from_file(
                        ctx.data->icon_path,
                        icon_size / 2, icon_size / 2);
                }
                pfi->bg_draw = true;
                pfi->padding_left = style.FramePadding.x + acc_width;
                pfi->padding_top = style.FramePadding.y + icon_size / 2;
                pfi->rounding = icon_size / 2;
            }


            // elements
            auto lbl_name = c->make_label(ctx.data->get_best_display_name());
            lbl_name->padding_left = icon_size + style.FramePadding.x * 3 + acc_width;
            lbl_name->padding_top = icon_size / 2 - 6 * scale;
            lbl_name->tooltip = tooltip;

            /*if(persist_popularity && ctx.data->popularity > 0) {
                c->same_line(this->width - style.FrameBorderSize * 4 - style.ItemSpacing.x * 8);
                auto cmd = c->make_button(fmt::format("{}", ctx.data->popularity), true, emphasis::error);
                cmd->tooltip = fmt::format("picked {}", str::humanise(ctx.data->popularity, "time", "times", "once", "twice"));
                cmd->bg_draw = true;
            }*/

            c->same_line();
            auto lbl_spacer = c->make_label(" ");
            lbl_spacer->padding_top = icon_size / 2;
            lbl_spacer->padding_left = -10 * scale;
        }, true);
        rpt->bind(choices);
        rpt->on_item_clicked = [this](size_t idx, shared_ptr<container> c, shared_ptr<browser_instance> bi) {
            launch(bi);
            if(persist_domain) {
                string rule_value = str::get_domain_from_url(up.open_url);
                bi->add_rule(rule_value);
                browser::persist_cache();
                ui::ensure_no_instances();
            }

            close();
        };

        cmd_cac->on_pressed = [this](button&) {
            win32::clipboard::set_ascii_text(up.url);
            close();
        };

        on_frame = [this](component&) {
            for(int i = 0; i < 9; i++) {
                ImGuiKey key = (ImGuiKey)(ImGuiKey_1 + i);
                if(ImGui::IsKeyDown(key)) {
                    launch(choices[i]);
                    close();
                }
            }
        };
    }

    void pick_window::launch(std::shared_ptr<browser_instance> bi) {
        bi->launch(up);
        bi->popularity += 1;
        g_config.set_popularity(bi->long_id(), bi->popularity);

    }
}