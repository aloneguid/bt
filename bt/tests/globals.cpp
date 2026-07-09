#include "../app/script_site.h"
#include "../app/url_pipeline.h"

#define CONFIG_NAME "bt_test"

// globals.h
bt::script_site g_script{grey::common::fss::get_config_file_path("bt_test", "scripts.lua"), true};
bt::state g_state{CONFIG_NAME};
grey::common::state_ticker<bt::state> g_state_ticker{g_state, 1.f};
bt::url_pipeline g_pipeline{g_state};