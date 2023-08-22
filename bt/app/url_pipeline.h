#pragma once
#include <string>
#include "url_pipeline_step.h"
#include <memory>
#include <vector>

namespace bt {
    /**
     * @brief URL processing pipeline. Includes stuff like cleaning the URL, unshortening etc.
    */
    class url_pipeline {
    public:
        url_pipeline();

        std::string process(const std::string& url);

        void reconfigure();

    private:
        std::vector<std::unique_ptr<url_pipeline_step>> steps;
    };
}