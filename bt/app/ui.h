#pragma once
#include <string>
#include <functional>
#include "browser.h"

namespace bt::ui
{
    enum class open_method {

        /// <summary>
        /// Force silent open in the default browser or first best match.
        /// No UI is ever shown.
        /// </summary>
        silent,

        /// <summary>
        /// If there are multiple options available, ask user for best.
        /// </summary>
        decide,

        /// <summary>
        /// Force pick, regardless of matching
        /// </summary>
        pick,

        // Use configured setting
        configured
    };

    using namespace std;

    extern std::function<void(bool is_open)> on_ui_open_changed;

    void set_main_instance();

    void render_ui_frame_if_required();

    void url_open(url_payload up, open_method method);

    void config();

    void url_logger();

    void ensure_no_instances();

    bool try_invoke_running_instance(const std::string& data);

    void contact();

    void coffee(const std::string& from);

    bool is_picker_hotkey_down();

    void send_anonymous_config();
}