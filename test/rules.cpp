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