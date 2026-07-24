#include "rule_hit_log.h"
#include <vector>
#include "datetime.h"
#include "common/fss.h"
#include "globals.h"
#include "magic_enum/magic_enum.hpp"

using namespace std;
namespace fs = std::filesystem;
using namespace grey::common;

namespace bt {
    rule_hit_log rule_hit_log::i;

    rule_hit_log::rule_hit_log() : 
        path{fss::get_config_file_path(CONFIG_NAME, "clicks.csv")},
        stream(path, ofstream::out | ofstream::app | ofstream::ate),
        writer(stream) {
        if(stream.tellp() == 0) {
            writer.write_row(vector<string> {
                "timestamp",

                // profile selection
                "browser",
                "profile",

                // click payload
                "url",
                "app_mode",
                "window_title",
                "process_path",
                "process_name",
                "process_description",

                "picker_invoked_reason",

                // match rule
                "rule_value",
                "rule_location",
                "rule_scope",
                "rule_is_regex",
                "rule_app_mode",
                "rule_is_fallback"
            });
            stream.flush();
        }
    }

    void rule_hit_log::write(const click_payload& up, const profile_selection& sel,
        optional<picker_invoked_reason> picker_reason,
        optional<browser_match_result> rule_match) {

        optional<match_rule> r = rule_match ? make_optional(rule_match->rule) : nullopt;

        writer.write_row(vector<string>{
            datetime::to_iso_8601(),

            // profile selection
            sel.b().name,
            sel.profile().name,

            // click payload
            up.url,
            up.app_mode ? "true" : "false",
            up.window_title,
            up.process_path,
            up.process_name,
            up.process_description,

            picker_reason ? string{magic_enum::enum_name(picker_reason.value())} : "",

            // match rule
            r ? r->value : "",
            r ? string{magic_enum::enum_name(r->loc)} : "",
            r ? string{magic_enum::enum_name(r->scope)} : "",
            r ? r->is_regex ? "true" : "false" : "",
            r ? r->app_mode ? "true" : "false" : "",
            r ? r->is_fallback ? "true" : "false" : ""
        });

        stream.flush();
    }
}