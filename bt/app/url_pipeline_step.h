#pragma once
#include <string>

namespace bt {
    class url_pipeline_step {
    public:
        virtual std::string process(const std::string& url) = 0;
    };
}