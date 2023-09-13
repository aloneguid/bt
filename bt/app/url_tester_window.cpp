#include "url_tester_window.h"
#include "../globals.h"

using namespace std;
using namespace grey;

namespace bt {
    url_tester_window::url_tester_window(grey::grey_context& ctx) : grey::window{ctx, "URL Tester", 600, 330} {
        can_resize = false;
        float scale = ctx.get_system_scale();
        make_label("Input:");
        auto txt_url = make_input(ICON_FA_GLOBE, &up.url);
        txt_url->tooltip = "Type your URL here to test";

        spacer();
        make_label("Results:");
        auto txt_clear_url = make_input(ICON_FA_GLOBE, &up.match_url);
        txt_clear_url->tooltip = "URL to apply rules to";

        auto txt_host = make_input(ICON_FA_LANDMARK_DOME, &u.host);
        txt_host->set_is_readonly();
        txt_host->width = 300;
        txt_host->tooltip = "Host name";

        same_line();
        auto txt_query = make_input(ICON_FA_LINES_LEANING, &u.query);
        txt_query->set_is_readonly();
        txt_query->tooltip = "Query string";

        auto txt_open_url = make_input(ICON_FA_GLOBE, &up.open_url);
        txt_open_url->tooltip = "URL to open in a browser.";

        spacer();
        make_label("Matches:");

        auto w_matches = make_child_window(0, 80 * scale);
        tbl = w_matches->make_complex_table<browser_match_result>({"rule", "browser", "profile"});
        tbl->stretchy = true;

        spacer();
        separator();
        auto cmd_close = make_button("Close");
        cmd_close->on_pressed = [this](button&) {
            close();
        };

        txt_url->on_value_changed = [this](string&) { match(); };

        match();
    }

    void url_tester_window::match() {
        up.match_url = up.open_url = "";
        g_pipeline.process(up);
        u = url{up.match_url};

        matches.clear();
        for(auto& m : browser::match(browser::get_cache(), up.match_url)) {
            matches.push_back(make_shared<browser_match_result>(m));
        }
        tbl->clear();

        for(auto& match : matches) {
            auto row = tbl->make_row(match);

            auto rc = row.cells[0];
            auto scope = rc->make_label(to_icon(match->rule.scope));
            scope->tooltip = "match scope";
            if(match->rule.priority != 0) {
                rc->same_line();
                auto p = rc->make_label(std::to_string(match->rule.priority));
                p->set_emphasis(emphasis::error);
                p->tooltip = "priority";
            }
            rc->same_line();
            rc->make_label(match->rule.value);

            row.cells[1]->make_label(match->bi->b->name);
            row.cells[2]->make_label(match->bi->name);
        }
    }

    std::string url_tester_window::to_icon(match_scope scope) {
        switch(scope) {
            case bt::match_scope::any:
                return ICON_FA_GLOBE;
            case bt::match_scope::domain:
                return ICON_FA_LANDMARK_DOME;
            case bt::match_scope::path:
                return ICON_FA_LINES_LEANING;
            default:
                return "?";
        }
    }
}