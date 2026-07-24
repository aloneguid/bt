#pragma once

namespace bt {
    enum class icon_overlay_mode : unsigned int {
        profile_on_browser  = 0,
        browser_on_profile  = 1,
        browser_only        = 2,
        profile_only        = 3
    };

    enum class picker_invoked_reason {
        not_invoked,
        hotkey_down,
        rule_conflict,
        no_rule_defined,
        forced
    };
}