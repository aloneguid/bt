#include "rule_hit_log.h"
#include "config.h"
#include <filesystem>
#include "fss.h"
#include "win32/shell.h"
#include "../globals.h"
#include <vector>
#include "datetime.h"

using namespace std;
namespace fs = std::filesystem;

namespace bt {

    const string FileName = "hit_log.csv";
    rule_hit_log rule_hit_log::i;

    rule_hit_log::rule_hit_log() : 
        path{config::get_data_file_path(FileName)},
        stream(path, ofstream::out | ofstream::app | ofstream::ate),
        writer(stream) {
        if(stream.tellp() == 0) {
            writer.write_row(vector<string> {
                "timestamp",
                "browser_id",
                "browser_name",
                "profile_name",
                "url",
                "rule",
                "calling_process_name",
                "calling_process_window_title"
            });
            stream.flush();
        }
    }

    void rule_hit_log::write(const bt::url_payload& url, const bt::browser_match_result& bmr) {
        writer.write_row(vector<string>{
            datetime::to_iso_8601(),
            bmr.bi->b->id,
            bmr.bi->b->name,
            bmr.bi->name,
            url.url,
            bmr.rule.to_line(),
            url.process_name,
            url.window_title
        });
        stream.flush();
    }
}