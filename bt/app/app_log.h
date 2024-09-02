#pragma once
#include <string>
#include <fstream>

namespace bt {
    class app_log {
    public:
        app_log();

        void write(const std::string& message);

        std::string get_absolute_path() { return path; }

        // global instance
        static app_log i;

    private:
        std::string path;
        std::ofstream stream;
    };
}