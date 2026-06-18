#include "../examples/auth_sample/login_action.hpp"

#include <drogular/action_context.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

drogular::ActionContext makeLoginContext(
    const std::string& body,
    drogular::ApplicationServices& services
) {
    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setMethod(drogon::Post);
    request->setContentTypeCode(
        drogon::CT_APPLICATION_X_FORM
    );
    request->setBody(body);

    return drogular::ActionContext(
        request,
        &services
    );
}

} // namespace

TEST(AuthSampleLoginActionTests, LogsInValidUser) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto context =
        makeLoginContext(
            "username=admin&password=password",
            services
        );

    LoginAction action;

    const auto result =
        action.handle(context);

    EXPECT_EQ(
        result.type(),
        drogular::ActionResultType::Redirect
    );

    EXPECT_EQ(
        result.location(),
        "/dashboard"
    );

    auto store =
        services.requireService<AuthStore>();

    ASSERT_TRUE(
        store->currentUser.value()
            .has_value()
    );

    EXPECT_EQ(
        store->currentUser.value()->username,
        "admin"
    );
}

TEST(AuthSampleLoginActionTests, RejectsInvalidUser) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto context =
        makeLoginContext(
            "username=admin&password=wrong",
            services
        );

    LoginAction action;

    const auto result =
        action.handle(context);

    EXPECT_EQ(result.location(), "/login");

    auto store =
        services.requireService<AuthStore>();

    EXPECT_FALSE(
        store->currentUser.value()
            .has_value()
    );
}