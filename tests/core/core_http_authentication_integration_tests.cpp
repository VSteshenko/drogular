#include <drogular/detail/request_context_state.hpp>
#include <drogular/security/session_authentication_adapter.hpp>
#include <drogular/security/principal_builder.hpp>
#include <drogular/session_store.hpp>
#include <drogular/services.hpp>

#include "../../src/http_security_integration.hpp"

#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace drogular;
using namespace drogular::security;

class HeaderAuthenticationAdapter final : public AuthenticationAdapter {
public:
    AuthenticationResult authenticate(const AuthenticationRequest& request) const override {
        const auto header = request.header("x-test-identity");
        if (!header) {
            return std::optional<SecurityContext>{};
        }
        if (*header == "reject") {
            return Unexpected{AuthenticationError{AuthenticationErrorCode::InvalidCredentials}};
        }
        if (*header == "unavailable") {
            return Unexpected{AuthenticationError{AuthenticationErrorCode::ProviderUnavailable}};
        }
        auto principal = PrincipalBuilder{}
            .issuer("integration-test")
            .subject(std::string(*header))
            .role("editor")
            .claim("permissions", std::vector<std::string>{"reports:read"})
            .build();
        if (!principal) {
            return Unexpected{AuthenticationError{AuthenticationErrorCode::InvalidCredentials}};
        }

        return std::optional<SecurityContext>{SecurityContext::forPrincipal(
            std::move(principal.value()), std::string{"header-test"})};
    }
};

class AlwaysAuthenticateAdapter final : public AuthenticationAdapter {
public:
    AuthenticationResult authenticate(const AuthenticationRequest&) const override {
        auto principal = PrincipalBuilder{}
            .issuer("second-adapter")
            .subject("fallback")
            .build();
        return std::optional<SecurityContext>{SecurityContext::forPrincipal(
            std::move(principal.value()))};
    }
};

std::shared_ptr<detail::RequestContextState> requestState(
    const drogon::HttpRequestPtr& request,
    ApplicationServices* services = nullptr
) {
    return std::make_shared<detail::RequestContextState>(request, services);
}

HttpSecurityConfiguration withHeaderAdapter(bool fallback = false) {
    auto pipeline = std::make_shared<AuthenticationPipeline>();
    pipeline->add(std::make_shared<HeaderAuthenticationAdapter>());
    if (fallback) {
        pipeline->add(std::make_shared<AlwaysAuthenticateAdapter>());
    }
    return {pipeline};
}

} // namespace

TEST(CoreHttpAuthenticationIntegration, AnonymousProtectedRouteReturns401) {
    auto request = drogon::HttpRequest::newHttpRequest();
    auto state = requestState(request);
    const auto rejected = detail::authenticateAndAuthorize(
        state, withHeaderAdapter(), HttpSecurityPolicy::authenticated());
    ASSERT_TRUE(rejected);
    EXPECT_EQ(*rejected, 401);
    EXPECT_FALSE(state->securityContext().authenticated());
}

TEST(CoreHttpAuthenticationIntegration, VerifiedHeaderPrincipalReachesRequestScope) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->addHeader("x-test-identity", "alice");
    auto state = requestState(request);
    const auto configuration = withHeaderAdapter();
    EXPECT_FALSE(detail::authenticateAndAuthorize(
        state, configuration, HttpSecurityPolicy::role("editor")));
    const auto principal = state->securityContext().principal();
    ASSERT_TRUE(principal);
    EXPECT_EQ(principal->id().subject, "alice");
    EXPECT_EQ(state->securityContext().authenticationMethod(),
              std::optional<std::string_view>{"header-test"});
    EXPECT_EQ(detail::authenticateAndAuthorize(
        state, configuration, HttpSecurityPolicy::permission("reports:read")),
        std::nullopt);
    EXPECT_EQ(detail::authenticateAndAuthorize(
        state, configuration, HttpSecurityPolicy::permission("reports:write")),
        std::optional<int>{403});
}

TEST(CoreHttpAuthenticationIntegration, BadCredentialCannotFallBackToOtherAdapter) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->addHeader("x-test-identity", "reject");
    auto state = requestState(request);
    EXPECT_EQ(detail::authenticateAndAuthorize(
        state, withHeaderAdapter(true), HttpSecurityPolicy{}),
        std::optional<int>{401});
    EXPECT_FALSE(state->securityContext().authenticated());
}

TEST(CoreHttpAuthenticationIntegration, ProviderUnavailableReturns503OnPublicRoute) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->addHeader("x-test-identity", "unavailable");
    auto state = requestState(request);
    EXPECT_EQ(detail::authenticateAndAuthorize(
        state, withHeaderAdapter(), HttpSecurityPolicy{}),
        std::optional<int>{503});
}

TEST(CoreHttpAuthenticationIntegration, ServerSessionResolvesAndCannotGrantArbitraryRoles) {
    ApplicationServices services;
    auto store = services.add<SessionStore>(ServiceLifetime::Singleton);
    auto session = store->create();
    session->set("username", "session-user");
    session->set("role", "admin");
    const auto id = session->get("_id");
    ASSERT_TRUE(id);

    auto pipeline = std::make_shared<AuthenticationPipeline>();
    pipeline->add(std::make_shared<SessionAuthenticationAdapter>("session-issuer"));
    HttpSecurityConfiguration configuration{pipeline};

    auto request = drogon::HttpRequest::newHttpRequest();
    request->addCookie("session_id", *id);
    auto state = requestState(request, &services);
    EXPECT_FALSE(detail::authenticateAndAuthorize(
        state, configuration, HttpSecurityPolicy::authenticated()));
    ASSERT_TRUE(state->securityContext().principal());
    EXPECT_EQ(state->securityContext().principal()->id().subject, "session-user");
    EXPECT_EQ(detail::authenticateAndAuthorize(
        state, configuration, HttpSecurityPolicy::role("admin")),
        std::optional<int>{403});

    auto forged = drogon::HttpRequest::newHttpRequest();
    forged->addCookie("session_id", "nonexistent-session-id");
    auto forgedState = requestState(forged, &services);
    EXPECT_EQ(detail::authenticateAndAuthorize(
        forgedState, configuration, HttpSecurityPolicy::authenticated()),
        std::optional<int>{401});
    EXPECT_FALSE(forgedState->securityContext().authenticated());
}

TEST(CoreHttpAuthenticationIntegration, IdentityNeverLeaksAcrossRequests) {
    auto authenticated = drogon::HttpRequest::newHttpRequest();
    authenticated->addHeader("x-test-identity", "alice");
    auto anonymous = drogon::HttpRequest::newHttpRequest();
    const auto configuration = withHeaderAdapter();
    auto first = requestState(authenticated);
    auto second = requestState(anonymous);
    EXPECT_FALSE(detail::authenticateAndAuthorize(first, configuration, {}));
    EXPECT_FALSE(detail::authenticateAndAuthorize(second, configuration, {}));
    EXPECT_TRUE(first->securityContext().authenticated());
    EXPECT_FALSE(second->securityContext().authenticated());
}

TEST(CoreHttpAuthenticationIntegration, ErrorResponsesDoNotExposeDetailsOrPermitCaching) {
    for (int status : {401, 403, 500, 503}) {
        const auto response = detail::securityErrorResponse(status);
        ASSERT_TRUE(response);
        EXPECT_EQ(static_cast<int>(response->statusCode()), status);
        EXPECT_EQ(response->getHeader("Cache-Control"), "no-store");
        EXPECT_EQ(response->body().find("session-user"), std::string::npos);
        EXPECT_EQ(response->body().find("x-test-identity"), std::string::npos);
    }
}