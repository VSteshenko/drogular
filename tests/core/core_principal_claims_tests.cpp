#include <drogular/security/principal_builder.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

using namespace drogular::security;

TEST(CorePrincipalClaims, TypedValuesAndSeparateNamespaces) {
    auto result = PrincipalBuilder{}
        .issuer("issuer").subject("subject")
        .claim("active", true)
        .claim("level", std::int64_t{3})
        .claim("score", 1.5)
        .claim("name", std::string{"example"})
        .claim("groups", std::vector<std::string>{"one", "two"})
        .attribute("locale", std::string{"de"})
        .attribute("active", false)
        .build();
    ASSERT_TRUE(result);

    const auto& principal = result.value();
    EXPECT_EQ(principal.claims().get<bool>("active"), true);
    EXPECT_EQ(principal.claims().get<std::int64_t>("level"), 3);
    EXPECT_EQ(principal.claims().get<double>("score"), 1.5);
    EXPECT_EQ(principal.claims().get<std::string>("name"), "example");
    EXPECT_EQ(principal.claims().get<std::vector<std::string>>("groups"),
              (std::vector<std::string>{"one", "two"}));
    EXPECT_EQ(principal.attributes().get<std::string>("locale"), "de");
    EXPECT_EQ(principal.attributes().get<bool>("active"), false);
    EXPECT_FALSE(principal.claims().contains("locale"));
    EXPECT_FALSE(principal.claims().get<std::string>("active"));
    EXPECT_FALSE(principal.claims().get<std::int64_t>("score"));
    EXPECT_EQ(principal.claims().find("missing"), nullptr);
    static_assert(std::is_same_v<decltype(std::declval<const Principal&>().claims()), const Claims&>);
    static_assert(std::is_same_v<decltype(std::declval<const Principal&>().attributes()), const Attributes&>);
}

TEST(CorePrincipalClaims, RejectsBadNamesAndDuplicates) {
    auto make = [] {
        return PrincipalBuilder{}
            .issuer("issuer")
            .subject("subject");
    };

    EXPECT_EQ(
        make()
            .claim(" ", true)
            .build()
            .error()
            .code,
        PrincipalErrorCode::InvalidClaimName);
    EXPECT_EQ(
        make()
            .attribute("", true)
            .build()
            .error()
            .code,
        PrincipalErrorCode::InvalidAttributeName);
    EXPECT_EQ(
        make()
            .claim("x", true)
            .claim("x", false)
            .build()
            .error()
            .code,
        PrincipalErrorCode::DuplicateClaim);
    EXPECT_EQ(
        make()
            .attribute("x", true)
            .attribute("x", false)
            .build()
            .error()
            .code,
        PrincipalErrorCode::DuplicateAttribute);
}

TEST(CorePrincipalClaims, ChecksIssuerSubjectClaims) {
    auto make = [] {
        return PrincipalBuilder{}.issuer("issuer").subject("subject");
    };

    EXPECT_TRUE(
        make()
            .claim("iss", std::string{"issuer"})
            .claim("sub", std::string{"subject"})
            .build());
    EXPECT_EQ(
        make()
            .claim("sub", std::string{"other"})
            .build()
            .error()
            .code,
        PrincipalErrorCode::ConflictingIdentityClaim);
    EXPECT_EQ(
        make()
            .claim("iss", true)
            .build()
            .error()
            .code,
        PrincipalErrorCode::ConflictingIdentityClaim);
}

TEST(CorePrincipalClaims, CopiesAreIndependentAndReadOnly) {
    auto result = PrincipalBuilder{}
        .issuer("issuer")
        .subject("subject")
        .claim("role", std::string{"viewer"})
        .build();
    ASSERT_TRUE(result);

    auto copy = result.value();
    EXPECT_EQ(copy.claims().get<std::string>("role"), "viewer");
    EXPECT_EQ(copy.id(), result->id());
}