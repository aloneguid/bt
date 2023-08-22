#include <gtest/gtest.h>
#include "../common/url.h"

using namespace std;

TEST(URL, Perfect) {

    url u{"https://host/query"};

    EXPECT_EQ("https", u.protocol);
    EXPECT_EQ("host", u.host);
    EXPECT_EQ("/query", u.query);
}

TEST(URL, NoProtocol) {

    url u{"host/query"};

    EXPECT_EQ("", u.protocol);
    EXPECT_EQ("host", u.host);
    EXPECT_EQ("/query", u.query);
}

TEST(URL, NoQuery) {

    url u{"https://host"};

    EXPECT_EQ("https", u.protocol);
    EXPECT_EQ("host", u.host);
    EXPECT_EQ("", u.query);
}