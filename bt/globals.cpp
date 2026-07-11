#include "globals.h"

// globals.h
bt::script_site g_script{grey::common::fss::get_config_file_path(CONFIG_NAME, "scripts.lua"), true};
bt::state_container g_state_container{CONFIG_NAME};
bt::state& g_state{g_state_container.get_state()};
grey::common::state_ticker<bt::state> g_state_ticker{g_state_container, 1.f};
bt::url_pipeline g_pipeline{g_state};