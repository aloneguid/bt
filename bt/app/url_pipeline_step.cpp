#include "url_pipeline_step.h"

namespace bt {
    std::string url_pipeline_step::to_string(url_pipeline_step_type type) {
        switch(type) {
            case url_pipeline_step_type::rule_matcher: return "Match rule";
            case url_pipeline_step_type::find_replace: return "Find/replace";
            case url_pipeline_step_type::o365: return "Unwrap Office 365";
            case url_pipeline_step_type::unshortener: return "Un-shorten";
            case url_pipeline_step_type::script: return "Script";
            default: return "unknown";
        }
    }
}