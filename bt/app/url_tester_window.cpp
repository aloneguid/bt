#include "url_tester_window.h"
#include "../globals.h"

using namespace std;
using namespace grey;

namespace bt {
    url_tester_window::url_tester_window(grey::grey_context& ctx) : grey::window{ctx, "URL Tester", 600, 400} {
        can_resize = false;
        float scale = ctx.get_system_scale();
        make_label("Input:");

        //auto lst_loc = make_listbox("");
        //lst_loc->mode = listbox_mode::icons;
        //lst_loc->items.push_back(list_item{ICON_FK_GLOBE, "URL"});
        //lst_loc->items.push_back(list_item{ICON_FK_WINDOW_MAXIMIZE, "Window title"});
        //lst_loc->items.push_back(list_item{ICON_FK_MICROCHIP, "Process name"});
        //lst_loc->selected_index = 0;
        //same_line();

        auto txt_url = make_input("URL", &up.url);
        auto txt_title = make_input("window title", &up.window_title);
        auto txt_proc = make_input("process name", &up.process_name);

        spacer();
        make_label("Results:");
        auto txt_clear_url = make_input(ICON_FK_QUESTION, &up.match_url);
        txt_clear_url->tooltip = "URL to apply rules to";
        txt_clear_url->set_is_readonly();

        auto txt_host = make_input("/", &u.host);
        txt_host->set_is_readonly();
        txt_host->width = 300;
        txt_host->tooltip = "Host name";

        same_line();
        auto txt_query = make_input("", &u.query);
        txt_query->set_is_readonly();
        txt_query->tooltip = "Query string";

        auto txt_open_url = make_input(ICON_FK_EXTERNAL_LINK, &up.open_url);
        txt_open_url->set_is_readonly();
        txt_open_url->tooltip = "URL to open in a browser.";

        spacer();
        make_label("Matches:");

        w_matches = make_child_window(0, 100 * scale);
        w_matches->has_border = true;

        spacer();
        separator();
        auto cmd_close = make_button("Close");
        cmd_close->set_emphasis(emphasis::primary);
        cmd_close->on_pressed = [this](button&) {
            close();
        };

        txt_url->on_value_changed = txt_title->on_value_changed = txt_proc->on_value_changed = [this](string&) { match(); };

        match();
    }

    void url_tester_window::match() {
        up.match_url = up.open_url = "";
        g_pipeline.process(up);
        u = url{up.match_url};

        matches.clear();
        for(auto& m : browser::match(browser::get_cache(), up)) {
            matches.push_back(make_shared<browser_match_result>(m));
        }
        w_matches->clear();

        for(auto& match : matches) {

            auto icon = w_matches->make_label(ICON_FK_WINDOW_MAXIMIZE);
            icon->set_emphasis(emphasis::error);

            w_matches->same_line();
            auto lbl_browser = w_matches->make_label(match->bi->b->name);

            w_matches->same_line();
            w_matches->make_label("|")->is_enabled = false;

            w_matches->same_line();
            auto lbl_profile = w_matches->make_label(match->bi->name);

            w_matches->make_label("");
            w_matches->same_line(35 * scale);
            w_matches->make_label(match->rule.to_string());
            w_matches->spacer();
            w_matches->separator();

            //rc->make_label(match->rule.to_string());

            //row.cells[1]->make_label(match->bi->b->name);
            //row.cells[2]->make_label(match->bi->name);
        }
    }

    std::string url_tester_window::to_icon(match_scope scope) {
        switch(scope) {
            case bt::match_scope::any:
                return ICON_FK_GLOBE;
            case bt::match_scope::domain:
                return ICON_FK_ACTIVITYPUB;
            case bt::match_scope::path:
                return ICON_FK_ACTIVITYPUB;
            default:
                return "?";
        }
    }
}