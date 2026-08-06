#include <gtest/gtest.h>
#include "../app/pipeline/o365.h"
#include "../app/click_payload.h"

using namespace bt;
using namespace bt::pipeline;

TEST(O365Test, StandardOutlookSafeLink) {
    click_payload cp;
    cp.url = "https://test.safelinks.protection.outlook.com/?url=https%3A%2F%2Fexample.com&data=extra";
    
    o365 step;
    step.process(cp);
    
    EXPECT_EQ(cp.url, "https://example.com");
}

TEST(O365Test, TeamsSafeLinkWithData) {
    click_payload cp;
    cp.url = "https://statics.teams.cdn.office.net/?data=https%3A%2F%2Fexample.net";
    
    o365 step;
    step.process(cp);
    
    EXPECT_EQ(cp.url, "https://example.net");
}

TEST(O365Test, NonSafeLink) {
    click_payload cp;
    cp.url = "https://google.com";
    
    o365 step;
    step.process(cp);
    
    EXPECT_EQ(cp.url, "https://google.com");
}
