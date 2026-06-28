#include <drogular/url.hpp>

#include <gtest/gtest.h>

TEST(CoreUrlTests, LeavesSafeCharactersUnchanged) {
    EXPECT_EQ(
        drogular::Url::encode("abcXYZ-_.~123"),
        "abcXYZ-_.~123"
    );
}

TEST(CoreUrlTests, EncodesSpaces) {
    EXPECT_EQ(
        drogular::Url::encode("hello world"),
        "hello%20world"
    );
}

TEST(CoreUrlTests, EncodesQuerySeparators) {
    EXPECT_EQ(
        drogular::Url::encode("a&b=c"),
        "a%26b%3Dc"
    );
}

TEST(CoreUrlTests, EncodesUtf8Bytes) {
    EXPECT_EQ(
        drogular::Url::encode("ä"),
        "%C3%A4"
    );
}