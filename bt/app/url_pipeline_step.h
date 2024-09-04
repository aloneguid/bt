#pragma once
#include <string>
#include "click_payload.h"

namespace bt {

    /**
     * @brief To avoid any kind of reflection, we use this enum to identify the type of a pipeline step.
    */
    enum class url_pipeline_step_type {
        rule_matcher = 0,   // match a rule using the configured browser rules
        find_replace,
        o365,
        unshortener,
        script
    };

    class url_pipeline_step {
    public:

        const url_pipeline_step_type type;

        virtual void process(click_payload& up) = 0;

        /**
         * @brief Convert to human-readable string.
        */
        static std::string to_string(url_pipeline_step_type type);

    protected:
        url_pipeline_step(url_pipeline_step_type type)
            : type{type} {}
    };
}