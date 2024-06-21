#pragma once
#include "grey.h"
#include "browser.h"
#include "dash_window.h"
#include "url_tester_window.h"
#include "url_pipeline_window.h"
#include "config.h"

namespace bt
{
    class config_window : public grey::window {
    public:
        config_window(grey::grey_context& gctx);
        void init();

    private:
        grey::grey_context& gctx;
        float scale;
        float last_updated_sec{1.0f};
        bool is_editing_profile{false};
        bool is_http{false};
        bool is_https{false};
        bool is_xbt{false};
        bool log_rule_hits{false};
        bool show_hidden_browsers{true};
        std::vector<std::shared_ptr<bt::browser>> browsers;
        std::shared_ptr<browser_instance> fb;
        std::string latest_version;
        std::shared_ptr<grey::menu_bar> menu;

        std::shared_ptr<dash_window> w_dash;
        bool dash_visible{false};

        bool panel_no_browsers_visible{false};
        bool panel_left_visible{false};
        bool panel_right_visible{false};
        std::shared_ptr<grey::repeater<browser>> rpt_browsers;
        std::shared_ptr<grey::child> browser_toolbar;
        std::shared_ptr<grey::tabs> profiles_tabs;
        bool profiles_tabs_visible{true};
        std::shared_ptr<grey::child> browser_free_area;
        bool browser_free_area_visible{true};

        std::shared_ptr<grey::label> st_health;

        std::shared_ptr<grey::menu_item> mi_fallback;
        std::shared_ptr<grey::menu_item> mi_om_silent;
        std::shared_ptr<grey::menu_item> mi_om_decide;
        std::shared_ptr<grey::menu_item> mi_om_pick;
        std::shared_ptr<grey::menu_item> mi_phk_never;
        std::shared_ptr<grey::menu_item> mi_phk_ctrlshift;
        std::shared_ptr<grey::menu_item> mi_phk_ctrlalt;
        std::shared_ptr<grey::menu_item> mi_phk_altshift;
        std::shared_ptr<grey::menu_item> mi_ff_mode_off;
        std::shared_ptr<grey::menu_item> mi_ff_mode_bt;
        std::shared_ptr<grey::menu_item> mi_ff_mode_ouic;

        bool demo_visible{false};
        
        void build_menu();
        void build_default_browser_menu();
        void build_status_bar();
        void refresh_proto_status(std::shared_ptr<grey::label> lbl, bool is);

        void handle_selection(std::shared_ptr<bt::browser> b);
        void handle_menu_click(grey::menu_item& mi);
        void build_browsers();
        void rediscover_browsers();
        void add_custom_browser_by_asking();
        void bind_edit_rules(std::shared_ptr<grey::container> parent, std::shared_ptr<browser_instance> bi);

        void render(std::shared_ptr<bt::browser> b, std::shared_ptr<container> c);

        void set_fallback(std::shared_ptr<browser_instance> bi);

        void set_open_method(const std::string& update_to);
        void set_picker_hotkey(const std::string& update_to);
        void update_firefox_mode(bool update, firefox_container_mode mode);

        void update_health_icon(bool healthy);
        void persist_ui();

        size_t index_of(std::shared_ptr<bt::browser> b);
    };
}