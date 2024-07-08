#pragma once
#include <string>
#include <memory>
#include <vector>
#include "url_pipeline_step.h"
#include "config.h"
#include "pipeline/replacer.h"

namespace bt {
    /**
     * @brief URL processing pipeline. Includes stuff like cleaning the URL, unshortening, find/replace etc.
    */
    class url_pipeline {
    public:
        url_pipeline(config& cfg);

        void process(url_payload& up);

        /**
         * @brief Reloads pipeline from configuration file.
        */
        void load();

        std::vector<std::shared_ptr<url_pipeline_step>>& get_steps() { return steps; }

        /**
         * @brief Get replacer step by index by finding the replacer step among all steps and downcasting it.
         * @param idx 
         * @return 
         */
        std::shared_ptr<bt::pipeline::replacer> get_replacer_step(size_t idx);

    private:
        config& cfg;
        std::vector<std::shared_ptr<url_pipeline_step>> steps;
    };
}