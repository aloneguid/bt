#include "url_tester_window.h"
#include "discovery.h"
#include "config.h"

using namespace std;
using namespace grey;

namespace bt {
    url_tester_window::url_tester_window(grey::grey_context& ctx) : grey::window{ctx, "URL Tester", 500, 380} {
        can_resize = false;
        float scale = ctx.get_system_scale();
        make_label("Input URL");
        auto txt_url = make_input("Input URL");
        txt_url->lines = 3;

        spacer();
        make_input("protocol", &proto)->is_enabled = false;
        make_input("domain", &host)->is_enabled = false;
        make_input("path", &path)->is_enabled = false;

        spacer();
        make_label("Matches:");
        auto w_matches = make_child_window(600 * scale, 100 * scale);
        auto tbl = w_matches->make_complex_table<browser_match_result>({"rule", "browser", "profile"});
        tbl->stretchy = true;

        txt_url->on_value_changed = [this, tbl](string& s) {

            match_rule::parse_url(s, proto, host, path);

            string cu;
            matches.clear();
            for(auto& m : browser::match(browser::get_cache(), s, cu)) {
                matches.push_back(make_shared<browser_match_result>(m));
            }
            tbl->clear();

            for(auto& match : matches) {
                auto row = tbl->make_row(match);
                row.cells[0]->make_label(match->rule.to_string());
                row.cells[1]->make_label(match->bi->b->name);
                row.cells[2]->make_label(match->bi->name);
            }
        };

        /*url->on_value_changed = [this](string& url_text) {
            url_tester_results->nodes.clear();

            string url;
            auto matches = browser::match(discovery::get_all_browsers(), url_text, url);

            for(auto& match : matches) {
                auto node = url_tester_results->add_node(match.bi->display_name, true, false);
                auto rule = node->make_input("rule");
                rule->set_value(match.rule.value);
                rule->is_enabled = false;
            }
        };*/

        separator();
        auto cmd_close = make_button("Close");
        cmd_close->on_pressed = [this](button&) {
            close();
        };

    }
}