#pragma once
#include <string>

namespace bt::app {
    bool has_new_version(std::string& latest_version_number);

    bool should_check_new_version();
}