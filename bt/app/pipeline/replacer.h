#pragma once
#include <string>
#include <vector>
#include "../url_pipeline_step.h"

namespace bt::pipeline {

    enum class replacer_kind : size_t {
        find_replace = 0,
        regex
    };

    class replacer : public url_pipeline_step {
    public:

        replacer_kind kind{replacer_kind::find_replace};
        std::string find;
        std::string replace;

        replacer(replacer_kind kind, const std::string& find, const std::string& replace);
        replacer(const std::string& rule);

        std::string serialise();

        // Inherited via url_pipeline_step
        void process(url_payload& up) override;

    private:
    };
}