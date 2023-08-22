#pragma once
#include <string>
#include <memory>
#include <vector>
#include "url_pipeline_step.h"
#include "config.h"

namespace bt {
    /**
     * @brief URL processing pipeline. Includes stuff like cleaning the URL, unshortening etc.
    */
    class url_pipeline {
    public:
        url_pipeline(config& cfg);

        std::string process(const std::string& url);

        void reconfigure();

    private:
        config& cfg;
        std::vector<std::unique_ptr<url_pipeline_step>> steps;
    };
}