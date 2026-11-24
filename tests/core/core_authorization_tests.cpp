#include <drogular/security/authorization.hpp>
#include <drogular/security/principal_builder.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace drogular::security;

namespace {

SecurityContext makeContext(std::vector<std::string> permissions = {"projects:read"}) {
    auto result = PrincipalBuilder{}
        .issuer("test.issuer")
        .subject("user-1")
        .role("editor")
        .claim("permissions", std::move(permissions))
        .build();
    EXPECT_TRUE(result.has_value());
    return SecurityContext::forPrincipal(std::move(result.value()));
}

} // namespace

TEST(CoreAuthorization, AnonymousRequiresAuthentication) {
    const auto context = SecurityContext::anonymous();
    EXPECT_EQ(
        Authorization::requireAuthenticated(context),
        AuthorizationDecision::AuthenticationRequired);
    EXPECT_EQ(
        Authorization::requireRole(context, "editor"),
        AuthorizationDecision::AuthenticationRequired);
    EXPECT_EQ(
        Authorization::requirePermission(context, "projects:read"),
        AuthorizationDecision::AuthenticationRequired);
}

TEST(CoreAuthorization, AuthenticatedDoesNotImplicitlyGrantAccess) {
    const auto context = makeContext();
    EXPECT_EQ(
        Authorization::requireAuthenticated(context),
        AuthorizationDecision::Allowed);
    EXPECT_EQ(
        Authorization::requireRole(context, "admin"),
        AuthorizationDecision::Forbidden);
    EXPECT_EQ(
        Authorization::requirePermission(context, "projects:write"),
        AuthorizationDecision::Forbidden);
}

TEST(CoreAuthorization, RequiresExactRolesAndPermissions) {
    const auto context = makeContext();
    EXPECT_EQ(
        Authorization::requireRole(context, "editor"),
        AuthorizationDecision::Allowed);
    EXPECT_EQ(
        Authorization::requireRole(context, "Editor"),
        AuthorizationDecision::Forbidden);
    EXPECT_EQ(
        Authorization::requireRole(context, ""),
        AuthorizationDecision::Forbidden);
    EXPECT_EQ(
        Authorization::requirePermission(context, "projects:read"),
        AuthorizationDecision::Allowed);
    EXPECT_EQ(
        Authorization::requirePermission(context, "projects:*"),
        AuthorizationDecision::Forbidden);
    EXPECT_EQ(
        Authorization::requirePermission(context, ""),
        AuthorizationDecision::Forbidden);
}

TEST(CoreAuthorization, MissingOrWrongTypedPermissionClaimFailsClosed) {
    auto result = PrincipalBuilder{}
        .issuer("test.issuer")
        .subject("user-2")
        .role("editor")
        .claim("permissions", std::string{"projects:read"})
        .build();
    ASSERT_TRUE(result);
    auto wrongType = SecurityContext::forPrincipal(std::move(result.value()));
    EXPECT_EQ(
        Authorization::requirePermission(wrongType, "projects:read"),
        AuthorizationDecision::Forbidden);

    auto noClaims = PrincipalBuilder{}
        .issuer("test.issuer")
        .subject("user-3")
        .build();
    ASSERT_TRUE(noClaims);
    EXPECT_EQ(
        Authorization::requirePermission(SecurityContext::forPrincipal(std::move(noClaims.value())),"projects:read"),
        AuthorizationDecision::Forbidden);
}

TEST(CoreAuthorization, PermissionNeverAutomaticallyFollowsRole) {
    auto result = PrincipalBuilder{}
        .issuer("test.issuer")
        .subject("user-4")
        .role("admin")
        .build();
    ASSERT_TRUE(result);
    auto context = SecurityContext::forPrincipal(std::move(result.value()));
    EXPECT_EQ(Authorization::requireRole(context, "admin"), AuthorizationDecision::Allowed);
    EXPECT_EQ(Authorization::requirePermission(context, "admin"), AuthorizationDecision::Forbidden);
}

TEST(CoreAuthorization, DetachedContextKeepsAuthorizationSnapshot) {
    SecurityContext detached;
    {
        auto original = makeContext();
        detached = original;
    }
    EXPECT_EQ(
        Authorization::requirePermission(detached, "projects:read"),
        AuthorizationDecision::Allowed);
}