#include "update_check.h"
#include <win32/http.h>
#include "config.h"
#include "../globals.h"
#include <str.h>

using namespace std;
using namespace std::chrono;

namespace bt::app {
    bool has_new_version(std::string& latest_version_number) {
        win32::http h;
        string latest_version = h.get("www.aloneguid.uk", "/projects/bt/bin/latest.txt");
        str::trim(latest_version);
        bool has_update = !latest_version.empty() && latest_version != APP_VERSION;
        latest_version_number = has_update ? latest_version : APP_VERSION;
        return has_update;
    }

    bool should_check_new_version() {
        auto last_checked = bt::config::i.get_last_update_check_time();
        auto diff = system_clock::now() - last_checked;
        auto hours = duration_cast<std::chrono::hours>(diff).count();
        return hours >= 24;
    }
}