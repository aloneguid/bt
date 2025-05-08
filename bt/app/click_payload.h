#pragma once
#include <string>
#if WIN32
#include <Windows.h>
#endif

namespace bt {
    struct click_payload {
        std::string url;

        bool app_mode{false};

#if WIN32
        HWND source_window_handle;  // handle of the system window where the click came from
#else
        void* source_window_handle;
#endif

        // everything below is populated from HWND
        std::string window_title;
        std::string process_name;
        std::string process_description;

        bool empty() const {
            return url.empty() && window_title.empty() && process_name.empty();
        }

        void clear(bool leave_url = false) {
            if(!leave_url) {
                url.clear();
            }
            window_title.clear();
            process_name.clear();
        }
    };
}