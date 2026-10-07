#include "rule_hit_log.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include "datetime.h"
#include "common/fss.h"
#include "globals.h"
#include "magic_enum/magic_enum.hpp"

using namespace std;
namespace fs = std::filesystem;
using namespace grey::common;

namespace bt {
    namespace {
        tm local_time() {
            const time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
            tm result{};
#if defined(_WIN32)
            localtime_s(&result, &now);
#else
            localtime_r(&now, &result);
#endif
            return result;
        }

        string date_string(const tm& date, const char* format) {
            ostringstream result;
            result << put_time(&date, format);
            return result.str();
        }

        string current_rotation_date() {
            return date_string(local_time(), "%Y%m%d");
        }

        string current_log_path() {
            return fss::get_config_file_path(CONFIG_NAME,
                "clicks_" + date_string(local_time(), "%Y%m") + ".csv");
        }
    }

    rule_hit_log rule_hit_log::i;

    rule_hit_log::rule_hit_log() : 
        path{current_log_path()},
        rotation_date{current_rotation_date()},
        stream(path, ofstream::out | ofstream::app | ofstream::ate),
        writer(stream) {
        write_header();
    }

    void rule_hit_log::write_header() {
        if(stream.tellp() != 0) return;

        writer.write_row(vector<string> {
            "timestamp",

            // profile selection
            "browser",
            "profile",

            // click payload
            "raw_url",
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

    void rule_hit_log::rotate_if_needed() {
        const string date = current_rotation_date();
        if(date == rotation_date) return;

        stream.close();
        path = current_log_path();
        stream.open(path, ofstream::out | ofstream::app | ofstream::ate);
        rotation_date = date;
        write_header();
    }

    void rule_hit_log::write(const click_payload& up, const profile_selection& sel,
        optional<picker_invoked_reason> picker_reason,
        optional<browser_match_result> rule_match) {

        rotate_if_needed();

        optional<match_rule> r = rule_match ? make_optional(rule_match->rule) : nullopt;

        writer.write_row(vector<string>{
            datetime::to_iso_8601(),

            // profile selection
            sel.b().name,
            sel.p().name,

            // click payload
            up.raw_url,
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