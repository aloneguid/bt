#pragma once
#include "lua.hpp"
#include <string>
#include "url_payload.h"

namespace bt {
    class script_site {
    public:
        script_site(const std::string& path_or_code, bool is_path);
        ~script_site();

        void reload();

        std::string get_path() const { return is_path ? path_or_code : ""; }
        std::string get_error() const { return error; }

        // code as string manipulation
        std::string get_code() const { return code; }
        void set_code(const std::string& code);

        // bt specific functions
        bool call_rule(url_payload& up, const std::string& function_name);


    private:
        bool is_path;
        std::string path_or_code;
        std::string code;
        std::string error;
        lua_State* L{nullptr};
    };
}