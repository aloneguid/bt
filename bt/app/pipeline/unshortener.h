#pragma once
#include "../url_pipeline_step.h"
#include "win32/http.h"

namespace bt::pipeline {
    class unshortener : public bt::url_pipeline_step {

    public:
        unshortener() : url_pipeline_step(url_pipeline_step_type::unshortener, "Unshorten") {}

        // Inherited via url_pipeline_step
        void process(click_payload& up) override;

    private:
        win32::http http;

        bool is_supported(const std::string& abs_url);
    };
}