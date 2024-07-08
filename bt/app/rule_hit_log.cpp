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
                "match_url",
                "open_url",
                "rule",
                "calling_process_name",
                "calling_process_window_title"
            });
            stream.flush();
        }
    }

    void rule_hit_log::write(const bt::url_payload& up, std::shared_ptr<bt::browser_instance> bi, const std::string& rule) {
        writer.write_row(vector<string>{
            datetime::to_iso_8601(),
            bi->b->id,
            bi->b->name,
            bi->name,
            up.url,
            up.match_url,
            up.open_url,
            rule,
            up.process_name,
            up.window_title
        });
        stream.flush();
    }
}