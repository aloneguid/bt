#include <gtest/gtest.h>
#include "common/url.h"

using namespace std;
using namespace grey::common;

namespace {
    void expect_parts(const url& actual, const string& scheme, const string& host, const string& port,
                      const string& path, const string& query) {
        EXPECT_EQ(scheme, actual.scheme);
        EXPECT_EQ(host, actual.host);
        EXPECT_EQ(port, actual.port);
        EXPECT_EQ(path, actual.path);
        EXPECT_EQ(query, actual.query);
    }
}

TEST(URL, ParsesAllComponentPermutations) {
    struct test_case {
        string input;
        string scheme;
        string host;
        string port;
        string path;
        string query;
    };

    const test_case cases[] = {
        {"", "", "", "", "", ""},
        {"https://host", "https", "host", "", "", ""},
        {"https://host:8080", "https", "host", "8080", "", ""},
        {"https://host/path", "https", "host", "", "/path", ""},
        {"https://host:8080/path", "https", "host", "8080", "/path", ""},
        {"https://host?key=value", "https", "host", "", "", "key=value"},
        {"https://host:8080/path?key=value", "https", "host", "8080", "/path", "key=value"},
        {"custom+scheme.1://user:password@host:443/a/b?x=1", "custom+scheme.1", "host", "443", "/a/b",
         "x=1"},
        {"//host", "", "host", "", "", ""},
        {"//host:8080/path?key=value", "", "host", "8080", "/path", "key=value"},
        {"https://[2001:db8::1]:8443/path", "https", "[2001:db8::1]", "8443", "/path", ""},
        {"mailto:user@example.com", "mailto", "", "", "user@example.com", ""},
        {"relative/path", "", "", "", "relative/path", ""},
        {"relative/path?key=value", "", "", "", "relative/path", "key=value"},
        {"/rooted/path", "", "", "", "/rooted/path", ""},
        {"\\rooted\\path?key=value", "", "", "", "\\rooted\\path", "key=value"},
    };

    for(const auto& test : cases) {
        SCOPED_TRACE(test.input);
        url actual{test.input};
        expect_parts(actual, test.scheme, test.host, test.port, test.path, test.query);
    }
}

TEST(URL, ParsesWindowsAndUncPaths) {
    {
        url actual{"C:"};
        expect_parts(actual, "", "", "", "C:", "");
        EXPECT_EQ("C:", actual.to_string());
    }
    {
        url actual{"C:\\software\\some_app.exe"};
        expect_parts(actual, "", "", "", "C:\\software\\some_app.exe", "");
        EXPECT_EQ("C:\\software\\some_app.exe", actual.to_string());
    }
    {
        url actual{"C:/software/some_app.exe"};
        expect_parts(actual, "", "", "", "C:/software/some_app.exe", "");
        EXPECT_EQ("C:/software/some_app.exe", actual.to_string());
    }
    {
        url actual{"\\\\server\\share\\file.txt"};
        expect_parts(actual, "", "server", "", "\\share\\file.txt", "");
        EXPECT_EQ("\\\\server\\share\\file.txt", actual.to_string());
    }
    {
        url actual{"\\\\server/share/file.txt"};
        expect_parts(actual, "", "server", "", "/share/file.txt", "");
        EXPECT_EQ("//server/share/file.txt", actual.to_string());
    }
    {
        url actual{"\\\\server"};
        expect_parts(actual, "", "server", "", "", "");
        EXPECT_EQ("//server", actual.to_string());
    }
}

TEST(URL, ParsesOpaqueAndInvalidSchemes) {
    {
        url actual{"tel:+1-555-0100"};
        expect_parts(actual, "tel", "", "", "+1-555-0100", "");
        EXPECT_EQ("tel:+1-555-0100", actual.to_string());
    }
    {
        url actual{"urn:isbn:9780131103627"};
        expect_parts(actual, "urn", "", "", "isbn:9780131103627", "");
        EXPECT_EQ("urn:isbn:9780131103627", actual.to_string());
    }
    {
        url actual{"a:b"};
        expect_parts(actual, "", "", "", "a:b", "");
        EXPECT_EQ("a:b", actual.to_string());
    }
    {
        url actual{"scheme_name:value"};
        expect_parts(actual, "", "", "", "scheme_name:value", "");
        EXPECT_EQ("scheme_name:value", actual.to_string());
    }
    {
        url actual{"https:///path"};
        expect_parts(actual, "https", "", "", "/path", "");
        EXPECT_EQ("https:/path", actual.to_string());
    }
}

TEST(URL, ParsesQueryParameters) {
    url actual{"https://host/path?name=John+Doe&encoded=%2Ftmp%2Ffile&flag&empty=&duplicate=first&duplicate=second"};

    expect_parts(actual, "https", "host", "", "/path",
                 "name=John+Doe&encoded=%2Ftmp%2Ffile&flag&empty=&duplicate=first&duplicate=second");
    EXPECT_EQ((unordered_map<string, string>{{"name", "John Doe"},
                                             {"encoded", "/tmp/file"},
                                             {"flag", ""},
                                             {"empty", ""},
                                             {"duplicate", "first"}}),
              actual.parameters);
}

TEST(URL, ToleratesMalformedQueryPairsAndEscapes) {
    url actual{"/path?&&missing_equals&=value&bad%2G=still%ZZ&trailing="};

    EXPECT_EQ("/path", actual.path);
    EXPECT_EQ("&&missing_equals&=value&bad%2G=still%ZZ&trailing=", actual.query);
    EXPECT_EQ((unordered_map<string, string>{{"missing_equals", ""},
                                             {"", "value"},
                                             {"bad%2G", "still%ZZ"},
                                             {"trailing", ""}}),
              actual.parameters);
}

TEST(URL, FragmentsAreNotPartOfPathOrQuery) {
    url with_query{"https://host/path?key=value#fragment"};
    expect_parts(with_query, "https", "host", "", "/path", "key=value");
    EXPECT_EQ("https://host/path?key=value", with_query.to_string());

    url without_query{"https://host/path#fragment"};
    expect_parts(without_query, "https", "host", "", "/path", "");
    EXPECT_EQ("https://host/path", without_query.to_string());
}

TEST(URL, ToStringPreservesEveryParsedForm) {
    const pair<string, string> cases[] = {
        {"https://host", "https://host"},
        {"https://host:8080/path", "https://host:8080/path"},
        {"//host/path", "//host/path"},
        {"\\\\server\\share\\file.txt", "\\\\server\\share\\file.txt"},
        {"C:\\software\\some_app.exe", "C:\\software\\some_app.exe"},
        {"/software/some_app", "/software/some_app"},
        {"relative/path", "relative/path"},
        {"mailto:user@example.com", "mailto:user@example.com"},
        {"https://host/path?query=hello+world", "https://host/path?query=hello+world"},
    };

    for(const auto& [input, expected] : cases) {
        SCOPED_TRACE(input);
        EXPECT_EQ(expected, url{input}.to_string());
    }
}

TEST(URL, ToStringSerializesManuallySetParameters) {
    url actual{"https://host/path"};
    actual.parameters.emplace("search", "hello world&url");

    EXPECT_EQ("https://host/path?search=hello+world%26url", actual.to_string());
}

TEST(URL, ToStringUsesRawQueryBeforeDecodedParameters) {
    url actual{"https://host/path?raw=%2f+value"};
    actual.parameters.emplace("different", "value");

    EXPECT_EQ("https://host/path?raw=%2f+value", actual.to_string());
}

TEST(URL, ToStringSupportsAllComponentCombinations) {
    {
        url actual{""};
        actual.scheme = "https";
        actual.host = "host";
        actual.port = "443";
        actual.path = "/path";
        actual.query = "key=value";
        EXPECT_EQ("https://host:443/path?key=value", actual.to_string());
    }
    {
        url actual{""};
        actual.scheme = "mailto";
        actual.path = "user@example.com";
        EXPECT_EQ("mailto:user@example.com", actual.to_string());
    }
    {
        url actual{""};
        actual.host = "host";
        actual.path = "/path";
        EXPECT_EQ("//host/path", actual.to_string());
    }
    {
        url actual{""};
        actual.host = "server";
        actual.path = "\\share";
        EXPECT_EQ("\\\\server\\share", actual.to_string());
    }
    {
        url actual{""};
        actual.path = "relative/path";
        EXPECT_EQ("relative/path", actual.to_string());
    }
}