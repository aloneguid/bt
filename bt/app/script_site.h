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

        std::vector<std::string> all_function_names;    // all function names
        std::vector<std::string> bt_function_names;     // all function names which in some way are relevant to business logic
        std::vector<std::string> ppl_function_names;    // all function names which are relevant to pipeline processing
        std::vector<std::string> rule_function_names;   // all function names which are relevant to rule processing

        void reload();

        std::string get_path() const { return is_path ? path_or_code : ""; }
        std::string get_error() const { return error; }

        // code as string manipulation
        std::string get_code() const { return code; }
        void set_code(const std::string& code);

        // bt specific functions

        bool call_rule(const click_payload& up, const std::string& function_name);

        std::string call_ppl(const click_payload& up, const std::string& function_name);

        void handle_lua_print(const std::string& msg);

    private:
        bool is_path;
        std::string path_or_code;
        std::string code;
        std::string error;
        lua_State* L{nullptr};

        void lua_push(const click_payload& up);

        void discover_function_names();
    };
}