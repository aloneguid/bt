#pragma once
#include "grey.h"
#include "../browser.h"

/**
 * @brief Extra Grey widgets, specific to Browser Tamer
 */
namespace bt::ui {
    void btw_on_app_initialised(grey::app& app);

    void btw_icon(grey::app& app, std::shared_ptr<bt::browser_instance> bi, float padding, float icon_size, bool is_active);
}