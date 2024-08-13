#pragma once
#include <string>
#include "../url_pipeline_step.h"

namespace bt::pipeline {
    class o365 : public url_pipeline_step {
    public:
        o365() : url_pipeline_step(url_pipeline_step_type::o365) {}

        // Inherited via url_pipeline_step
        void process(click_payload& up) override;
    };
}