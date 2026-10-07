#pragma once
#include "../url_pipeline_step.h"

namespace bt::pipeline {
    class script : public bt::url_pipeline_step {
    public:
        script(const std::string& function_name) : 
            function_name{function_name}, url_pipeline_step(url_pipeline_step_type::script) {}

        const std::string function_name;

        // Inherited via url_pipeline_step
        void process(click_payload& up) override;
    };
}