#include "app_log.h"
#include "config.h"
#include "datetime.h"

using namespace std;
namespace fs = std::filesystem;

namespace bt {

    const string AppLogFileName = "log.txt";
    app_log app_log::i;

    app_log::app_log() :
        path{config::get_data_file_path(AppLogFileName)},
        stream(path, ofstream::out | ofstream::app | ofstream::ate) {
    }

    void app_log::write(const std::string& message) {
        auto ts = datetime::to_iso_8601();
        stream << ts << " " << message << std::endl;
        stream.flush();
    }
}