#include "url_tester_window.h"
#include "discovery.h"
#include "config.h"

using namespace std;
using namespace grey;

namespace bt {
    url_tester_window::url_tester_window(grey::grey_context& ctx) : grey::window{ctx, "URL Tester", 500, 350} {
        can_resize = false;
        float scale = ctx.get_system_scale();
        make_label("Input URL");
        auto txt_url = make_input("Input URL");

        spacer();
        make_input("host", &u.host)->is_enabled = false;
        make_input("query", &u.query)->is_enabled = false;

        spacer();
        make_label("Matches:");
        tbl = make_complex_table<browser_match_result>({"rule", "browser", "profile"});
        tbl->stretchy = true;

        txt_url->on_value_changed = [this](string& s) { match(s); };

        match("");
    }

    void url_tester_window::match(const string& s) {
        u = url{s};

        string cu;
        matches.clear();
        for(auto& m : browser::match(browser::get_cache(), s, cu)) {
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
                p->set_emphasis(emphasis::warning);
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