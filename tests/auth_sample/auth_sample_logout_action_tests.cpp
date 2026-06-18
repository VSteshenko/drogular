#include "../examples/auth_sample/logout_action.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

drogular::ActionContext makeLogoutContext(
    drogular::ApplicationServices& services
) {
    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setMethod(drogon::Post);

    return drogular::ActionContext(
        request,
        &services
    );
}

} // namespace

TEST(AuthSampleLogoutActionTests, ClearsAuthenticatedUser) {
    drogular::ApplicationServices services;

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto store =
        services.requireService<AuthStore>();

    store->currentUser.set(
        AuthUser{
            .id = 1,
            .username = "admin",
            .role = "admin"
        }
    );

    auto context =
        makeLogoutContext(services);

    LogoutAction action;

    action.handle(context);

    EXPECT_FALSE(
        store->currentUser.value()
            .has_value()
    );
}

TEST(AuthSampleLogoutActionTests, RedirectsToLogin) {
    drogular::ApplicationServices services;

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto context =
        makeLogoutContext(services);

    LogoutAction action;

    const auto result =
        action.handle(context);

    EXPECT_EQ(
        result.type(),
        drogular::ActionResultType::Redirect
    );

    EXPECT_EQ(
        result.location(),
        "/login"
    );
}