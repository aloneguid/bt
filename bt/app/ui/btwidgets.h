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
        float padding, float icon_size, bool is_active);

    void btw_icon(grey::app& app,
        const browser& b, const browser_profile& p,
        icon_overlay_mode icon_mode,
        float padding, float icon_size, bool is_active);
}