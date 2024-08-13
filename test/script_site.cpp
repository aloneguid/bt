#include <gtest/gtest.h>
#include "../bt/app/script_site.h"

using namespace std;
using namespace bt;

TEST(Script, Loads) {
    bt::script_site ss{R"()", false};
    EXPECT_EQ(ss.get_error(), "");
}

TEST(Script, SimpleMatch) {
    bt::script_site ss{R"(
function test1()
    return true;
end
)", false};

    EXPECT_EQ(ss.get_error(), "");

    click_payload up{"http://test.com"};
    bool matches = ss.call_rule(up, "test1");
    EXPECT_TRUE(matches);
}