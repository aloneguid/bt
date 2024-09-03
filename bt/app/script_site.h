#pragma once
#include "lua.hpp"
#include <string>
#include <vector>
#include <functional>
#include "click_payload.h"

namespace bt {
    class script_site {
    public:
        script_site(const std::string& path_or_code, bool is_path);
        ~script_site();

        std::string print_buffer;
        std::function<void(const std::string&)> on_print;

        void reload();

        std::string get_path() const { return is_path ? path_or_code : ""; }
        std::string get_error() const { return error; }

        // code as string manipulation
        std::string get_code() const { return code; }
        void set_code(const std::string& code);

        // bt specific functions

        bool call_rule(const click_payload& up, const std::string& function_name);

        std::string call_ppl(const click_payload& up, const std::string& function_name);

        /**
         * @brief Analyse the code and return a list of Lua function names with supported prefixes
         * @return 
         */
        std::vector<std::string> list_function_names();

        void handle_lua_print(const std::string& msg);

    private:
        bool is_path;
        std::string path_or_code;
        std::string code;
        std::string error;
        lua_State* L{nullptr};

        void lua_push(const click_payload& up);
    };
}