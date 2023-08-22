#pragma once
#include "../url_pipeline_step.h"
#include "win32/http.h"

namespace bt::pipeline {
    class unshortener : public bt::url_pipeline_step {

    public:
        // Inherited via url_pipeline_step
        std::string process(const std::string& url) override;

    private:
        win32::http http;

        bool is_supported(const std::string& abs_url);
    };
}