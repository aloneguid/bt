#include <gtest/gtest.h>
#include "../app/pipeline/unshortener.h"
#include "../app/pipeline/replacer.h"
#include "../app/click_payload.h"

using namespace std;
using namespace bt;
using namespace bt::pipeline;

TEST(UnshortenerTest, SupportedDomains) {
    unshortener step;
    EXPECT_TRUE(step.is_supported("https://bit.ly/47EZHSl"));
    EXPECT_TRUE(step.is_supported("https://BIT.LY/47EZHSl"));
    EXPECT_TRUE(step.is_supported("https://tinyurl.com/abc"));
    EXPECT_TRUE(step.is_supported("https://t.co/xyz"));
    EXPECT_TRUE(step.is_supported("https://is.gd/test"));
    EXPECT_TRUE(step.is_supported("https://cutt.ly/test"));
    EXPECT_FALSE(step.is_supported("https://github.com/aloneguid/bt"));
}

TEST(ReplacerTest, InvalidRegexDoesNotThrow) {
    click_payload cp;
    cp.url = "https://example.com/test";

    // Unclosed bracket is an invalid regex pattern
    replacer step{replacer_kind::regex, "[invalid", "replacement"};
    EXPECT_NO_THROW(step.process(cp));
    EXPECT_EQ(cp.url, "https://example.com/test");
}