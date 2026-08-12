#pragma once
#include "grey.h"
#include "model.h"
#include "../browser.h"

/**
 * @brief Extra Grey widgets, specific to Browser Tamer
 */
namespace bt::ui {
    void btw_on_app_initialised(grey::app& app);

    void btw_icon(grey::app& app,
        const profile_selection& selection,
        float padding_x, float padding_y,
        float icon_size);

    void btw_icon(grey::app& app,
        const browser& b, const browser_profile& p,
        icon_overlay_mode icon_mode,
        float padding_x, float padding_y,
        float icon_size);

    void btw_rule(browser& b, browser_profile& bi, match_rule& rule, int i);
}