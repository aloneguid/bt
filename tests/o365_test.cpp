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

TEST(O365Test, TeamsSafeLinkWithUrl) {
    click_payload cp;
    cp.url = "https://teams.public.onecdn.static.microsoft/evergreen-assets/safelinks/2/atp-safelinks.html?url=https%3A%2F%2Fgithub.com%2Faloneguid%2Fbt&locale=en-us&dest=https%3A%2F%2Fteams.microsoft.com%2Fapi%2Fmt%2Fpart%2Famer-02%2Fbeta%2Fatpsafelinks%2Fgeturlreputationsitev2.";

    o365 step;
    step.process(cp);

    EXPECT_EQ(cp.url, "https://github.com/aloneguid/bt");
}

TEST(O365Test, NonSafeLink) {
    click_payload cp;
    cp.url = "https://google.com";
    
    o365 step;
    step.process(cp);
    
    EXPECT_EQ(cp.url, "https://google.com");
}
