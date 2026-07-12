#include "rule_hit_log.h"
#include <vector>
#include "datetime.h"
#include "common/fss.h"
#include "globals.h"

using namespace std;
namespace fs = std::filesystem;
using namespace grey::common;

namespace bt {
    rule_hit_log rule_hit_log::i;

    rule_hit_log::rule_hit_log() : 
        path{grey::common::fss::get_config_file_path(APP_SHORT_NAME, "hit_log.csv")},
        stream(path, ofstream::out | ofstream::app | ofstream::ate),
        writer(stream) {
        if(stream.tellp() == 0) {
            writer.write_row(vector<string> {
                "timestamp",
                "browser_id",
                "browser_name",
                "profile_name",
                "url",
                "n/a",
                "open_url",
                "rule",
                "calling_process_name",
                "calling_process_window_title"
            });
            stream.flush();
        }
    }

    void rule_hit_log::write(const click_payload& up, const profile_selection& sel, const std::string& rule) {
        writer.write_row(vector<string>{
            datetime::to_iso_8601(),
            sel.browser().name,
            sel.profile().name,
            up.url,
            "",
            up.url,
            rule,
            up.process_name,
            up.window_title
        });
        stream.flush();
    }
}