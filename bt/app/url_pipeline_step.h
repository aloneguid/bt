#pragma once
#include <string>
#include "url_payload.h"

namespace bt {

    /**
     * @brief To avoid any kind of reflection, we use this enum to identify the type of a pipeline step.
    */
    enum class url_pipeline_step_type {
        rule_matcher = 0,   // match a rule using the configured browser rules
        o365,
        unshortener,
        replacer
    };

    class url_pipeline_step {
    public:
        virtual void process(url_payload& up) = 0;
        const url_pipeline_step_type type;

    protected:
        url_pipeline_step(url_pipeline_step_type type) : type{type} {}
    };
}