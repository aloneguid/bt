#include "update_check.h"
#include <win32/http.h>
#include "config.h"
#include "../globals.h"
#include <str.h>
#include "ext/github.h"

using namespace std;
using namespace std::chrono;

namespace bt::app {
    bool has_new_version(std::string& latest_version_number) {
        ext::github gh;
        ext::github_release ghr = gh.get_latest_release("aloneguid", "bt");
        if(!ghr.is_valid) return false;

        string latest_version = ghr.tag;
        bool has_update = !latest_version.empty() && latest_version != APP_VERSION;
        latest_version_number = has_update ? latest_version : APP_VERSION;
        g_config.set_last_update_check_time_to_now();
        return has_update;
    }

    bool should_check_new_version() {
        auto last_checked = g_config.get_last_update_check_time();
        auto diff = system_clock::now() - last_checked;
        auto hours = duration_cast<std::chrono::hours>(diff).count();
        return hours >= 24;
    }
}