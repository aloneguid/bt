#pragma once
#include <string>
#include <Windows.h>

namespace bt {
    struct url_payload {
        std::string url;    // very first, raw input URL

        bool app_mode{false};

        HWND source_window_handle;  // handle of the system window where the click came from

        // everything below is populated from HWND
        std::string window_title;
        std::string process_name;

        // the URLs below are optional, and if populated, extra handling is possible
        std::string match_url;  // URL to match on
        std::string open_url;   // final URL to open
    };
}