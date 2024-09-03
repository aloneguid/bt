#include "script_site.h"
#include <fstream>
#include <regex>

using namespace std;

namespace bt {

    const string FileName = "rules.lua";

    script_site::script_site(const string& path_or_code, bool is_path) :
        path_or_code{path_or_code}, is_path{is_path} {
        reload();
    }

    script_site::~script_site() {
        if(L) {
            lua_close(L);
        }
    }

    static int lua_print(lua_State* L) {
        // Get the script_site instance from the Lua registry
        script_site* instance = static_cast<script_site*>(lua_touserdata(L, lua_upvalueindex(1)));

        // Get the number of arguments
        int nargs = lua_gettop(L);

        // Collect all arguments into a single string
        std::string output;
        for(int i = 1; i <= nargs; ++i) {
            if(lua_isstring(L, i)) {
                output += lua_tostring(L, i);
            } else {
                output += "<non-string>";
            }
            if(i < nargs) {
                output += "\t";
            }
        }

        // Call the member function
        instance->handle_lua_print(output);

        return 0; // Number of return values
    }


    void script_site::reload() {
        error.clear();
        print_buffer.clear();

        // reinintialise interpreter
        if(L) {
            lua_close(L);
            L = nullptr;
        }

        L = luaL_newstate();
        luaL_openlibs(L);

        // Register the custom print function
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, lua_print, 1);
        lua_setglobal(L, "print");

        // load code into string
        {
            if(is_path) {
                ifstream fs(path_or_code);
                if(fs.is_open()) {
                    code = string(istreambuf_iterator<char>(fs), istreambuf_iterator<char>());
                }
            } else {
                code = path_or_code;
            }
        }

        // load code into interpreter
        if(luaL_loadstring(L, code.c_str()) || lua_pcall(L, 0, 0, 0)) {
            error = lua_tostring(L, -1);
            // pop error message from the stack
            lua_pop(L, 1);
            return;
        }
    }

    void script_site::set_code(const std::string& code) {
        // save code to file
        {
            if(is_path) {
                ofstream fs(path_or_code);
                if(fs.is_open()) {
                    fs << code;
                }
            }
        }

        reload();
    }

    bool script_site::call_rule(click_payload& up, const string& function_name) {

        // set global table "p" with 3 members: url, window_title, process_name
        lua_newtable(L);
        lua_pushstring(L, up.url.c_str());
        lua_setfield(L, -2, "url");
        lua_pushstring(L, up.window_title.c_str());
        lua_setfield(L, -2, "wt");
        lua_pushstring(L, up.process_name.c_str());
        lua_setfield(L, -2, "pn");
        lua_setglobal(L, "p");

        // call function
        lua_getglobal(L, function_name.c_str());
        if(lua_pcall(L, 0, 1, 0)) {
            // get error message from the stack
            error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return false;
        }

        // read return value
        bool r = lua_toboolean(L, -1);
        lua_pop(L, 1);

        // read back "url" from global table "p" (in case it's changed)
        lua_getglobal(L, "p");
        lua_getfield(L, -1, "url");
        //up.match_url = lua_tostring(L, -1);
        lua_pop(L, 1);

        return r;
    }

    std::vector<std::string> script_site::list_function_names() {
        vector<string> r;

        // use regex to list function names
        regex re{R"(function\s+(\w+))"};
        sregex_iterator it{code.begin(), code.end(), re};
        sregex_iterator end;
        for(; it != end; ++it) {
            r.push_back(it->str(1));
        }

        return r;
    }

    void script_site::handle_lua_print(const std::string& msg) {
        print_buffer += msg + "\n";
        if(on_print) {
            on_print(msg);
        }
    }
}