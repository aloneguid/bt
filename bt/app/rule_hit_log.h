#pragma once
#include <string>
#include "browser.h"
#include <optional>
#include <iterator> // due to bug in p-ranav/csv2
#include <csv2/writer.hpp>

#include "model.h"

namespace bt {
    class rule_hit_log {
    public:
        rule_hit_log();

        void write(const click_payload& up, const profile_selection& sel,
            std::optional<picker_invoked_reason> picker_reason,
            std::optional<browser_match_result> rule_match);

        std::string get_absolute_path() { return path; }

        // global instance
        static rule_hit_log i;

    private:
        std::string path;
        std::ofstream stream;
        csv2::Writer<csv2::delimiter<','>> writer;
    };
}
