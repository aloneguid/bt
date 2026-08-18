#include "btwidgets.h"
#include "../../globals.h"
#include "../../res.h"
#include "fss.h"
#include "../strings.h"
#include "stl.hpp"
#include "str.h"
#include "common/process.h"

using namespace std;
using namespace grey;
using namespace grey::common;
namespace w = grey::widgets;

std::vector<std::string> rule_locations { "URL", "Title", "Process", bt::strings::LuaScript };
std::vector<std::pair<std::string, std::string>> url_scopes{
    { ICON_MD_LANGUAGE, "Match anywhere" },
    { ICON_MD_GITE, "Match only in host name" },
    { ICON_MD_ROUNDABOUT_LEFT, "Match only in path" }
};

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
        float padding_x, float padding_y, float icon_size) {
        btw_icon(app, selection.b(), selection.profile(), g_state.icon_overlay, padding_x, padding_y, icon_size);

    }

    void btw_icon(grey::app &app, const browser &b, const browser_profile &p,
        icon_overlay_mode icon_mode,
        float padding_x, float padding_y,
        float icon_size) {

        ImVec2 box_size{icon_size + padding_x * 2, icon_size + padding_y * 2};

        w::group g;

        // render icon and come back to starting position
        float x0, y0;
        w::cur_get(x0, y0);

        w::dummy(box_size);

        w::cur_set(x0 + padding_x, y0 + padding_y);

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

        if(p.use_color || p.use_user_color) {
            // user color has priority
            unsigned color = p.use_user_color ? p.user_color : p.color;

            //draw circle around the icon with user color
            auto dl = ImGui::GetWindowDrawList();
            ImVec2 center{x0 + padding_x + icon_size / 2, y0 + padding_y + icon_size / 2};
            auto radius = icon_size / 2 + g_state.highlight_width / 2;
            //dl->AddCircle(center, radius, color, 0, g_state.highlight_width);
            dl->AddCircleFilled(center, radius, color);
        }

        w::image_rounded(app, icon1, icon_size, icon_size, icon_size / 2);

        // if required, draw overlay icon
        if(!icon2.empty()) {
            w::cur_set(x0 + padding_x + icon_size / 2, y0 + padding_y + icon_size / 2);
            float isz = icon_size / 2;
            w::image_rounded(app, icon2, isz, isz, isz);
        }
    }

    grey::widgets::popup pop_proc_names{"pop_proc_names"};
    std::vector<std::string> pop_proc_names_items;
    std::string pop_proc_names_filter;
    std::vector<std::string> pop_proc_names_items_filtered;
    unsigned int pop_proc_names_selected{0};

    void refresh_pop_proc_names_items() {
        auto procs = process::enumerate();
        pop_proc_names_items.clear();
        for(process &p: procs) {
            if(!p.is_valid()) continue;
            string name = p.get_name();
            if(!name.empty()) {
                pop_proc_names_items.push_back(name);
            }
        }

        // deduplicate and sort
        std::ranges::sort(pop_proc_names_items);
        pop_proc_names_items.erase(
            std::unique(pop_proc_names_items.begin(), pop_proc_names_items.end()),
            pop_proc_names_items.end());

        pop_proc_names_items_filtered = pop_proc_names_items;
        pop_proc_names_filter.clear();
    }


    void btw_rule( browser& b, browser_profile& bi, match_rule& rule, int i) {
        // location
        w::combo("##loc", rule_locations, reinterpret_cast<unsigned int&>(rule.loc), 90);

        // value
        w::sl();
        if(rule.loc == match_location::lua_script) {
            // get selected index
            unsigned int selected{0};
            for(unsigned int j = 0; j < g_script.rule_function_names.size(); j++) {
                if(g_script.rule_function_names[j] == rule.value) {
                    selected = j;
                    break;
                }
            }

            w::combo("##val", g_script.rule_function_names, selected, 250);
            w::tt(strings::LuaScriptTooltip);

            // reassign value
            if(!g_script.rule_function_names.empty()) {
                rule.value = g_script.rule_function_names[selected];
            }
        } else {
            w::input(rule.value, "##val", true, 250 * w::scale);
        }

        // up/down logic is very custom and is bound to the textbox itself
        if(ImGui::IsItemFocused()) {
            if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                stl::move(bi.rules, i, -1, true);
            } else if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                stl::move(bi.rules, i, 1, true);
            }
        }

        // is regex checkbox (not for Lua)
        if(rule.loc != match_location::lua_script) {
            w::sl();
            w::icon_checkbox(ICON_MD_GRAIN, rule.is_regex);
            w::tt(strings::RuleIsARegex);
        }

        // app mode
        if(b.supports_frameless_windows()) {
            w::sl();
            w::icon_checkbox(ICON_MD_TAB_UNSELECTED, rule.app_mode);
            w::tt("Open in chromeless window");
        }

        // scope (for "URL" rules)
        if(rule.loc == match_location::url) {
            w::sl();
            w::label("|", 0, false);
            w::sl();
            w::icon_list(url_scopes, reinterpret_cast<unsigned int&>(rule.scope));
        }

        // process name selection helper (for "process" rules)
        if(rule.loc == match_location::process_name) {
            w::sl();
            if(w::button(ICON_MD_DEVELOPER_BOARD)) {
                refresh_pop_proc_names_items();
                pop_proc_names.open();
            }
            w::tt(strings::RulePickProcessName);

            {
                w::guard gpop{pop_proc_names};
                if(pop_proc_names) {
                    if(w::input(pop_proc_names_filter, "##proc_filter")) {
                        pop_proc_names_selected = 0;
                        pop_proc_names_items_filtered.clear();
                        for(auto &p: pop_proc_names_items) {
                            if(str::contains_ic(p, pop_proc_names_filter)) {
                                pop_proc_names_items_filtered.push_back(p);
                            }
                        }
                    }
                    w::tt("Filter process names");
                    if(w::list("##proc", pop_proc_names_items_filtered, pop_proc_names_selected)) {
                        rule.value = pop_proc_names_items_filtered[pop_proc_names_selected];
                    }
                }
            }
        }

        w::sl();
        if(w::button(string{ICON_MD_DELETE} + "##" + to_string(i), emphasis::error)) {
            bi.delete_rule(rule.value);
        }
        w::tt("Delete rule");
    }
}
