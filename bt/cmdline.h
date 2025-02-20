#pragma once
#include <string>

class cmdline {
public:
    cmdline();

    int exec(const std::string& command, const std::string& data);

private:
    int exec_list();
    int exec_get_default();
    int exec_set_default(const std::string& data);
};