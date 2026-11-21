#include <drogular/security/principal_builder.hpp>

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

using namespace drogular::security;

TEST(CorePrincipal, BuildsUserWithImmutableReadAccess) {
    auto result = PrincipalBuilder{}
        .issuer("https://identity.example")
        .subject("user-123")
        .displayName("Example User")
        .role("editor")
        .role("editor")
        .role("reader")
        .build();
    ASSERT_TRUE(result);

    const auto& principal = result.value();
    EXPECT_EQ(principal.id().issuer, "https://identity.example");
    EXPECT_EQ(principal.id().subject, "user-123");
    EXPECT_EQ(principal.type(), PrincipalType::User);
    EXPECT_EQ(principal.displayName(), "Example User");
    EXPECT_EQ(principal.roles().size(), 2u);
    EXPECT_TRUE(principal.hasRole("reader"));
    EXPECT_FALSE(principal.hasRole("admin"));
    static_assert(std::is_same_v<decltype(std::declval<const Principal&>().id()),
                                          const PrincipalId&>);
    static_assert(std::is_same_v<decltype(std::declval<const Principal&>().roles()),
                                          const std::vector<std::string>&>);
}

TEST(CorePrincipal, IssuerScopesIdentity) {
    const PrincipalId first{"issuer-a", "user-123"};
    const PrincipalId second{"issuer-b", "user-123"};
    EXPECT_FALSE(first == second);
    EXPECT_TRUE(first == PrincipalId("issuer-a", "user-123"));
}

TEST(CorePrincipal, RejectsMissingIssuer) {
    auto result = PrincipalBuilder{}
        .subject("user").build();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, PrincipalErrorCode::MissingIssuer);
}

TEST(CorePrincipal, RejectsMissingSubject) {
    auto result = PrincipalBuilder{}
        .issuer("issuer")
        .subject("  ")
        .build();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, PrincipalErrorCode::MissingSubject);
}

TEST(CorePrincipal, RejectsBlankRole) {
    auto result = PrincipalBuilder{}
        .issuer("issuer")
        .subject("user")
        .role(" ")
        .build();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, PrincipalErrorCode::InvalidRole);
}

TEST(CorePrincipal, SupportsServiceIdentity) {
    auto result = PrincipalBuilder{}
        .issuer("system")
        .subject("worker")
        .type(PrincipalType::Service)
        .build();
    ASSERT_TRUE(result);
    EXPECT_EQ(result->type(), PrincipalType::Service);
    EXPECT_TRUE(result->roles().empty());
}

TEST(CorePrincipal, SupportsLvalueBuilder) {
    PrincipalBuilder builder;
    builder.issuer("issuer").subject("subject").role("user");
    auto result = std::move(builder).build();
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->hasRole("user"));
}