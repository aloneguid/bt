#pragma once
#include <string>
#include "url_payload.h"

namespace bt {
    class url_pipeline_step {
    public:
        virtual void process(url_payload& up) = 0;
    };
}