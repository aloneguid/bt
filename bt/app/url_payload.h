#pragma once
#include <string>
#if WIN32
#include <Windows.h>
#endif

namespace bt {
    struct url_payload {
        std::string url;    // very first, raw input URL

        bool app_mode{false};

#if WIN32
        HWND source_window_handle;  // handle of the system window where the click came from
#else
        void* source_window_handle;
#endif

        // everything below is populated from HWND
        std::string window_title;
        std::string process_name;

        // the URLs below are optional, and if populated, extra handling is possible
        std::string match_url;  // URL to match on
        std::string open_url;   // final URL to open

        bool empty() const {
            return url.empty() && window_title.empty() && process_name.empty();
        }

        void clear() {
            url.clear();
            window_title.clear();
            process_name.clear();
            match_url.clear();
            open_url.clear();
        }
    };
}