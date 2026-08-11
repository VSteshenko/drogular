#include "login_action.hpp"

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

    services.add<drogular::SessionStore>(
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

    EXPECT_EQ(result.location(), "/dashboard");

    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].name, "session_id");
    EXPECT_TRUE(result.cookies()[0].httpOnly);
    EXPECT_EQ(
        result.cookies()[0].sameSite,
        drogular::CookieSameSite::Lax
    );

    auto store =
        services.requireService<drogular::SessionStore>();

    const auto session =
        store->get(result.cookies()[0].value);

    ASSERT_NE(session, nullptr);

    EXPECT_EQ(
        session->get("username").value(),
        "admin"
    );

    EXPECT_EQ(
        session->get("role").value(),
        "admin"
    );
}

TEST(AuthSampleLoginActionTests, RejectsInvalidUser) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<drogular::SessionStore>(
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

    EXPECT_TRUE(result.cookies().empty());
}

TEST(AuthSampleLoginActionTests, RotatesExistingSessionOnLogin) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto store =
        services.requireService<drogular::SessionStore>();

    auto oldSession = store->create();
    oldSession->set("username", "user");
    oldSession->set("role", "user");

    const auto oldSessionId =
        oldSession->get("_id").value();

    auto context =
        makeLoginContext(
            "username=admin&password=password",
            services
        );

    context.request()->addCookie(
        "session_id",
        oldSessionId
    );

    LoginAction action;

    const auto result =
        action.handle(context);

    ASSERT_EQ(result.cookies().size(), 1);

    const auto newSessionId =
        result.cookies()[0].value;

    EXPECT_NE(newSessionId, oldSessionId);
    EXPECT_FALSE(store->contains(oldSessionId));
    EXPECT_TRUE(store->contains(newSessionId));

    const auto newSession =
        store->get(newSessionId);

    ASSERT_NE(newSession, nullptr);
    EXPECT_EQ(
        newSession->get("username").value(),
        "admin"
    );
    EXPECT_EQ(
        newSession->get("role").value(),
        "admin"
    );
}