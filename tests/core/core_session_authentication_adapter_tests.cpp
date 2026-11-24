#include <drogular/security/session_authentication_adapter.hpp>

#include <gtest/gtest.h>

using namespace drogular;
using namespace drogular::security;

TEST(CoreSessionAuthenticationAdapter, MissingSessionOrUsernameIsAnonymous) {
    SessionAuthenticationAdapter adapter{"app.session"};
    SessionAuthenticationRequest absent{nullptr};
    auto noSession = adapter.authenticate(absent);
    ASSERT_TRUE(noSession);
    EXPECT_FALSE(noSession.value().has_value());

    auto session = std::make_shared<Session>();
    SessionAuthenticationRequest noUser{session};
    auto result = adapter.authenticate(noUser);
    ASSERT_TRUE(result);
    EXPECT_FALSE(result.value().has_value());
}

TEST(CoreSessionAuthenticationAdapter, ExistingUsernameProducesPrincipal) {
    auto session = std::make_shared<Session>();
    session->set("username", "anna");
    session->set("role", "admin");
    SessionAuthenticationRequest request{session};
    AuthenticationPipeline pipeline;
    pipeline.add(std::make_shared<SessionAuthenticationAdapter>("my.app"));
    auto result = pipeline.authenticate(request);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->authenticated());
    EXPECT_EQ(result->principal()->id().issuer, "my.app");
    EXPECT_EQ(result->principal()->id().subject, "anna");
    EXPECT_TRUE(result->principal()->roles().empty());
    EXPECT_EQ(result->authenticationMethod(), std::optional<std::string_view>{"session"});
}

TEST(CoreSessionAuthenticationAdapter, EmptySubjectRejectedWithoutFallback) {
    auto session = std::make_shared<Session>();
    session->set("username", "");
    SessionAuthenticationRequest request{session};
    SessionAuthenticationAdapter adapter{"my.app"};
    auto result = adapter.authenticate(request);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, AuthenticationErrorCode::InvalidCredentials);
}

TEST(CoreSessionAuthenticationAdapter, RequestValueViewsRemainValid) {
    auto session = std::make_shared<Session>();
    session->set("username", "before");
    session->set("another", "data");
    SessionAuthenticationRequest request{session};
    const auto first = request.sessionValue("username");
    ASSERT_TRUE(first);
    session->set("username", "after");
    const auto second = request.sessionValue("another");
    ASSERT_TRUE(second);
    EXPECT_EQ(*first, "before");
    EXPECT_EQ(request.sessionValue("username"), std::optional<std::string_view>{"before"});
}

TEST(CoreSessionAuthenticationAdapter, IdentitySurvivesSessionLifetime) {
    auto session = std::make_shared<Session>();
    session->set("username", "worker");
    SessionAuthenticationRequest request{session};
    SessionAuthenticationAdapter adapter{"app.session"};
    auto result = adapter.authenticate(request);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result.value().has_value());
    const SecurityContext snapshot = *std::move(result.value());
    session.reset();
    EXPECT_EQ(snapshot.principal()->id().subject, "worker");
}

TEST(CoreSessionAuthenticationAdapter, ConfigurableSubjectKey) {
    auto session = std::make_shared<Session>();
    session->set("account_id", "42");
    SessionAuthenticationRequest request{session};
    SessionAuthenticationAdapter adapter{"app.session", "account_id"};
    auto result = adapter.authenticate(request);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result.value().has_value());
    EXPECT_EQ(result.value()->principal()->id().subject, "42");
}