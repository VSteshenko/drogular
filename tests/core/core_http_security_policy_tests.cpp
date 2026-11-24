#include <drogular/security/http_security_policy.hpp>
#include <drogular/security/principal_builder.hpp>

#include <gtest/gtest.h>

using namespace drogular::security;

namespace {

SecurityContext makeContext(bool editor = true) {
    auto builder = PrincipalBuilder{}.issuer("test").subject("alice");
    if (editor) {
        builder.role("editor");
    }
    builder.claim("permissions", std::vector<std::string>{"reports:read"});

    auto result = std::move(builder).build();
    EXPECT_TRUE(result.has_value());
    return SecurityContext::forPrincipal(std::move(result.value()));
}

}

TEST(CoreHttpSecurityPolicy, PublicRouteAllowsAnonymous) {
    EXPECT_EQ(evaluateHttpPolicy(SecurityContext::anonymous(), {}),
              AuthorizationDecision::Allowed);
}

TEST(CoreHttpSecurityPolicy, AuthenticatedRouteRejectsAnonymousWith401) {
    const auto decision = evaluateHttpPolicy(SecurityContext::anonymous(),
                                            HttpSecurityPolicy::authenticated());
    EXPECT_EQ(authorizationHttpStatus(decision), 401);
    EXPECT_EQ(evaluateHttpPolicy(makeContext(), HttpSecurityPolicy::authenticated()),
              AuthorizationDecision::Allowed);
}

TEST(CoreHttpSecurityPolicy, RolesAndPermissionsAreNotInterchangeable) {
    const auto context = makeContext();
    EXPECT_EQ(evaluateHttpPolicy(context, HttpSecurityPolicy::role("editor")),
              AuthorizationDecision::Allowed);
    EXPECT_EQ(evaluateHttpPolicy(context, HttpSecurityPolicy::permission("editor")),
              AuthorizationDecision::Forbidden);
    EXPECT_EQ(evaluateHttpPolicy(context, HttpSecurityPolicy::permission("reports:read")),
              AuthorizationDecision::Allowed);
    EXPECT_EQ(evaluateHttpPolicy(context, HttpSecurityPolicy::role("unknown")),
              AuthorizationDecision::Forbidden);
    EXPECT_EQ(authorizationHttpStatus(AuthorizationDecision::Forbidden), 403);
}

TEST(CoreHttpSecurityPolicy, AuthenticationErrorsNeverBecomeAnonymous) {
    EXPECT_EQ(authenticationErrorHttpStatus(AuthenticationErrorCode::InvalidCredentials), 401);
    EXPECT_EQ(authenticationErrorHttpStatus(AuthenticationErrorCode::ExpiredCredentials), 401);
    EXPECT_EQ(authenticationErrorHttpStatus(AuthenticationErrorCode::ProviderUnavailable), 503);
    EXPECT_EQ(authenticationErrorHttpStatus(AuthenticationErrorCode::InternalError), 500);
    EXPECT_EQ(authenticationErrorHttpStatus(AuthenticationErrorCode::InvalidAdapterResult), 500);
}