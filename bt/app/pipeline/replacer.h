#pragma once
#include <string>
#include <vector>
#include "../url_pipeline_step.h"

namespace bt::pipeline {

    enum class replacer_kind {
        contains = 0,
        regex
    };

    class replacer_rule {
    public:
        replacer_kind kind;
        std::string match;
        std::string replace;
    };

    class replacer : public url_pipeline_step {
    public:
        replacer(const std::vector<std::string>& rules);

        // Inherited via url_pipeline_step
        void process(url_payload& up) override;

    private:
        std::vector<replacer_rule> rules;
    };
}