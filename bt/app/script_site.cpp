#include "script_site.h"
#include <fstream>

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

    void script_site::reload() {
        error.clear();

        // reinintialise interpreter
        if(L) {
            lua_close(L);
            L = nullptr;
        }

        L = luaL_newstate();
        luaL_openlibs(L);

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

    bool script_site::call_rule(url_payload& up, const string& function_name) {

        // set global table "p" with 3 members: url, window_title, process_name
        lua_newtable(L);
        lua_pushstring(L, up.match_url.c_str());
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
}