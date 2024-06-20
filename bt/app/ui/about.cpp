#include "about.h"

using namespace std;
namespace w = grey::widgets;

namespace bt::ui {
    void about::run(float scale) {

        w::window wnd{"About"};
        wnd
            .size(310, 350, scale)
            .no_resize()
            .render();
    }
}