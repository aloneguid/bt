#include "globals.h"

// globals.h
bt::script_site g_script{grey::common::fss::get_config_file_path(CONFIG_NAME, "scripts.lua"), true};
bt::state g_state;
grey::common::config<bt::state> g_config{g_state, CONFIG_NAME};
bt::url_pipeline g_pipeline{g_state};