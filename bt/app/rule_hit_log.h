#pragma once
#include <string>
#include "browser.h"
#include <csv2/writer.hpp>

namespace bt {
    class rule_hit_log {
    public:
        rule_hit_log();

        void write(const bt::click_payload& up, std::shared_ptr<bt::browser_instance> bi, const std::string& rule);

        std::string get_absolute_path() { return path; }

        // global instance
        static rule_hit_log i;

    private:
        std::string path;
        std::ofstream stream;
        csv2::Writer<csv2::delimiter<','>> writer;
    };
}