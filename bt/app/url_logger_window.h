#pragma once
#include "grey.h"
#include "browser.h"
#include "../ext/lsignal/lsignal.h"

namespace bt {
    class url_logger_window : public grey::window, public lsignal::slot {
    public:
        explicit url_logger_window(grey::grey_context& ctx);
        ~url_logger_window() override;

        // signal receiver
        void operator()(const bt::url_payload&, const bt::browser_match_result&);

    private:
        const grey::grey_context& ctx;
        std::shared_ptr<grey::child> w_events;

        void append(const bt::url_payload& url, const bt::browser_match_result& bmr);
    };
}