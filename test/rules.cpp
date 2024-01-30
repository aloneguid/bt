#include <iostream>
#include <gtest/gtest.h>
#include "../bt/app/match_rule.h"

using namespace std;
using namespace bt;

TEST(Rules, MatchGlobalScope) {

    match_rule bmr{"blabla"};

    EXPECT_TRUE(bmr.is_match("http://noblabla.com/page.html"));
    EXPECT_FALSE(bmr.is_match("http://bla.com/page.html"));
}

TEST(Rules, MatchDomain) {

    match_rule bmr{"bla"};
    bmr.scope = match_scope::domain;

    EXPECT_TRUE(bmr.is_match("http://nobla.com/page.html"));
    EXPECT_FALSE(bmr.is_match("http://no.com/blapage.html"));
}

TEST(Rules, MatchPath) {

    match_rule bmr{"bla"};
    bmr.scope = match_scope::path;

    EXPECT_TRUE(bmr.is_match("http://no.com/blapage.html"));
    EXPECT_FALSE(bmr.is_match("http://nobla.com/page.html"));
}

TEST(Rules, MatchEmptyUrl) {

    match_rule bmr{"bla"};
    EXPECT_FALSE(bmr.is_match(""));

    bmr.scope = match_scope::domain;
    EXPECT_FALSE(bmr.is_match(""));

    bmr.scope = match_scope::path;
    EXPECT_FALSE(bmr.is_match(""));
}

TEST(Rules, MatchEmptyRule) {

    match_rule bmr{""};
    EXPECT_FALSE(bmr.is_match("x"));

    bmr.scope = match_scope::domain;
    EXPECT_FALSE(bmr.is_match("x"));

    bmr.scope = match_scope::path;
    EXPECT_FALSE(bmr.is_match("x"));
}

// --- serialisation ----

TEST(Rules, Serialise) {
    match_rule mr1{"r"};
    EXPECT_EQ("r", mr1.to_line());

    match_rule mr2{"r"};
    mr2.scope = match_scope::domain;
    EXPECT_EQ("scope:domain|r", mr2.to_line());

    match_rule mr3{"r"};
    mr2.scope = match_scope::path;
    EXPECT_EQ("scope:path|r", mr2.to_line());

    match_rule mr4{"p"};
    mr4.priority = 4;
    EXPECT_EQ("priority:4|p", mr4.to_line());

    match_rule mr5{"p"};
    mr5.app_mode = true;
    EXPECT_EQ("mode:app|p", mr5.to_line());
}

TEST(Rules, Deserialise) {
    match_rule mr1{"r"};
    EXPECT_EQ("r", mr1.value);
    EXPECT_EQ(match_scope::any, mr1.scope);

    match_rule mr11{"scope:any|r"};
    EXPECT_EQ("r", mr11.value);
    EXPECT_EQ(match_scope::any, mr11.scope);

    match_rule mr2{"scope:domain|r"};
    EXPECT_EQ("r", mr2.value);
    EXPECT_EQ(match_scope::domain, mr2.scope);

    match_rule mr3{"scope:path|r"};
    EXPECT_EQ("r", mr3.value);
    EXPECT_EQ(match_scope::path, mr3.scope);

    match_rule mr4{"priority:4|p"};
    EXPECT_EQ("p", mr4.value);
    EXPECT_EQ(4, mr4.priority);

    match_rule mr5{"mode:app|p"};
    EXPECT_EQ("p", mr5.value);
    EXPECT_EQ(true, mr5.app_mode);

    match_rule mr6{"just a co:lon"};
    EXPECT_EQ("just a co:lon", mr6.value);
    EXPECT_EQ(match_scope::any, mr6.scope);
}

// --- parse URL ---

TEST(Rules, ParseUrl) {
    string proto, host, path;

    match_rule::parse_url("github.com/user", proto, host, path);
    EXPECT_EQ("", proto);
    EXPECT_EQ("github.com", host);
    EXPECT_EQ("user", path);

    match_rule::parse_url("http://github.com/user", proto, host, path);
    EXPECT_EQ("http", proto);
    EXPECT_EQ("github.com", host);
    EXPECT_EQ("user", path);

    match_rule::parse_url("github.com", proto, host, path);
    EXPECT_EQ("", proto);
    EXPECT_EQ("github.com", host);
    EXPECT_EQ("", path);
}