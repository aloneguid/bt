#pragma once
#include "../url_pipeline_step.h"
#include "common/url.h"

namespace bt::pipeline {
    /**
     * Integrates with ClearURLs database.
     * Examples:
     * - https://www.amazon.com/dp/exampleProduct/ref=xsin
     */
    class clearurls : public url_pipeline_step {
    public:
        clearurls();

        void process(click_payload& cp) override;

    private:
        static void load_db();

        static grey::common::url clean(click_payload& cp, grey::common::url, int depth = 0);
    };
}