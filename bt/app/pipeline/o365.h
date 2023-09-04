#pragma once
#include <string>
#include "../url_pipeline_step.h"

namespace bt::pipeline {
    class o365 : public url_pipeline_step {
    public:
        // Inherited via url_pipeline_step
        void process(url_payload& up) override;
    };
}