#include <iostream>
#include <gtest/gtest.h>
#include "../app/match_rule.h"

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

TEST(Rules, ToString) {
    match_rule mr1{"r"};
    EXPECT_EQ("substring 'r' in URL", mr1.to_string());

    match_rule mr2{"r"};
    mr2.scope = match_scope::domain;
    EXPECT_EQ("substring 'r' in domain part of the URL", mr2.to_string());

    match_rule mr3{"r"};
    mr2.scope = match_scope::path;
    EXPECT_EQ("substring 'r' in query part of the URL", mr2.to_string());

    match_rule mr5{"p"};
    mr5.app_mode = true;
    EXPECT_EQ("substring 'p' in URL", mr5.to_string());
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