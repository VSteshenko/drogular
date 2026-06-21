#include "../examples/auth_sample/logout_action.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>
#include <drogular/session_store.hpp>

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

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto store =
        services.requireService<drogular::SessionStore>();

    auto session =
        store->create();

    const auto sessionId =
        session->get("_id").value();

    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->addCookie(
        "session_id",
        sessionId
    );

    drogular::ActionContext context(
        request,
        &services
    );

    LogoutAction action;

    action.handle(context);

    EXPECT_FALSE(
        store->contains(sessionId)
    );
}

TEST(AuthSampleLogoutActionTests, RedirectsToLogin) {
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

    LogoutAction action;

    const auto result =
        action.handle(context);

    EXPECT_EQ(
        result.location(),
        "/login"
    );

    ASSERT_EQ(
        result.cookies().size(),
        1
    );

    EXPECT_EQ(
        result.cookies()[0].name,
        "session_id"
    );

    EXPECT_EQ(
        result.cookies()[0].value,
        ""
    );
}