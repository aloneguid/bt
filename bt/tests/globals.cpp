#include "common/config.hpp"
#include "../app/script_site.h"
#include "../app/url_pipeline.h"

// globals.h
grey::common::config g_settings{"bt_test", "config.ini"};
bt::script_site g_script{grey::common::fss::get_config_file_path("bt_test", "scripts.lua"), true};
bt::url_pipeline g_pipeline{g_settings};