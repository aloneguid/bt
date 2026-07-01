#pragma once
#include "app/browser.h"
#include "common/config_handler.hpp"

namespace bt {
    void save_custom(grey::common::config_handler& cfg, std::vector<std::shared_ptr<bt::browser>>& browsers);

    void load_custom(grey::common::config_handler& cfg, std::vector<std::shared_ptr<bt::browser>>& browsers);
}