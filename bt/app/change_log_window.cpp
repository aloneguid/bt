#include "change_log_window.h"
#include "../log.h"
#include <fmt/core.h>

using namespace std;
using namespace grey;

namespace bt {
    change_log_window::change_log_window(grey::grey_context& ctx) : grey::window{ctx, "Change Log", 600, 400} {
        can_resize = false;
        center();
        float scale = ctx.get_system_scale();

        auto tree = make_tree();
        for(const auto& cv : ChangeLog) {
            auto node = tree->add_node(
                fmt::format("  {}", cv.number),
                true, true);
            node->is_bold = true;
            node->make_label("Released: ");
            node->same_line();
            auto d = node->make_label(cv.date);
            d->set_emphasis(emphasis::primary);

            if(!cv.news.empty()) {
                node->spacer();
                node->make_label("New Features");
                for(auto& item : cv.news) {
                    node->make_label(item, true)->text_wrap_pos = 500 * scale;
                }
            }

            if(!cv.improvements.empty()) {
                node->spacer();
                node->make_label("Improvements");
                for(auto& item : cv.improvements) {
                    node->make_label(item, true)->text_wrap_pos = 500 * scale;
                }
            }

            if(!cv.bugs.empty()) {
                node->spacer();
                node->make_label("Bugs Fixed");
                for(auto& item : cv.bugs) {
                    node->make_label(item, true)->text_wrap_pos = 500 * scale;
                }
            }
        }
    }
}