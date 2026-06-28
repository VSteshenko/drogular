#include <drogular/action_auth_support.hpp>
#include <drogular/action_context.hpp>
#include <drogular/session_store.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(CoreActionAuthSupportTests, RedirectsGuestToLogin) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, &services);

    const auto result =
        drogular::ActionAuthSupport::requireAuthentication(
            context
        );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(result.value().location(), "/login");
}

TEST(CoreActionAuthSupportTests, RedirectsGuestWhenSessionValueRequired) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, &services);

    const auto result =
        drogular::ActionAuthSupport::requireSessionValue(
            context,
            "role",
            "admin",
            "/users?error=access_denied"
        );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(result.value().location(), "/users?error=access_denied");
}

TEST(CoreActionAuthSupportTests, SupportsCustomLoginRedirect) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto request =
        drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(
        request,
        &services
    );

    const auto result =
        drogular::ActionAuthSupport::requireAuthentication(
            context,
            "/custom-login"
        );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(result.value().location(), "/custom-login");
}