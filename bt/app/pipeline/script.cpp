#include "script.h"
#include "../../globals.h"

namespace bt::pipeline {

    using namespace std;

    void script::process(click_payload& up) {
        string next = g_script.call_ppl(up, function_name);
        if(!next.empty()) {
            up.url = next;
        }
    }
}