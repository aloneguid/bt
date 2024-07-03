#pragma once
#include <string>
#include <memory>
#include <vector>
#include "url_pipeline_step.h"
#include "config.h"

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

        /**
         * @brief Saves pipeline to configuration file.
        */
        void save();

        std::vector<std::shared_ptr<url_pipeline_step>>& get_steps() { return steps; }

    private:
        config& cfg;
        std::vector<std::shared_ptr<url_pipeline_step>> steps;
    };
}