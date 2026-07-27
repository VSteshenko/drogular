#include "portal_application_test_host.hpp"
#include "support/html_test_support.hpp"

#include "data/demo_dataset.hpp"
#include "features/auth/actions/login_action.hpp"
#include "features/auth/actions/logout_action.hpp"
#include "features/auth/pages/login_page.hpp"
#include "features/auth/support/portal_auth_support.hpp"

#include <drogular/action_context.hpp>
#include <drogular/render_context.hpp>
#include <drogular/session_store.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

drogular::ActionContext makeActionContext(
    PortalApplicationTestHost& app,
    const std::unordered_map<std::string, std::string>& form = {},
    const std::string& sessionId = ""
) {
    auto request =
        drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Post);

    for (const auto& [name, value] : form) {
        request->setParameter(name, value);
    }

    if (!sessionId.empty()) {
        request->addCookie("session_id", sessionId);
    }

    return drogular::ActionContext(
        request,
        &app.services()
    );
}

drogular::RenderContext makeRenderContext(
    PortalApplicationTestHost& app,
    const std::string& sessionId = ""
) {
    auto request =
        drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Get);

    if (!sessionId.empty()) {
        request->addCookie("session_id", sessionId);
    }

    drogular::RenderContext context;
    context.setServices(&app.services());
    context.setRequest(request);
    return context;
}

std::string createSession(
    PortalApplicationTestHost& app,
    const std::string& username,
    const std::string& role
) {
    auto store =
        app.services().requireService<drogular::SessionStore>();

    auto session = store->create();
    session->set("username", username);
    session->set("role", role);

    return session->get("_id").value();
}

} // namespace

TEST(PortalAuthTests, LoginPageRendersForm) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto html = app.render<PortalLoginPage>();

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "<form method=\"post\" action=\"/login\">"
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "name=\"username\""
        )
    );
    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "name=\"password\""
        )
    );
    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            "Please enter username and password."
        )
    );
    EXPECT_FALSE(
        HtmlTestSupport::containsText(
            html,
            "Invalid username or password."
        )
    );
}

TEST(PortalAuthTests, LoginPageShowsMissingCredentialsError) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto html = app.render<PortalLoginPage>({
        {"error", "missing_credentials"}
    });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Please enter username and password."
        )
    );
}

TEST(PortalAuthTests, LoginPageShowsInvalidCredentialsError) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto html =
        app.render<PortalLoginPage>({
            {"error", "invalid_credentials"}
        });

    EXPECT_TRUE(
        HtmlTestSupport::containsText(
            html,
            "Invalid username or password."
        )
    );
}

TEST(PortalAuthTests, LoginRejectsMissingCredentials) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto result = app.post<PortalLoginAction>({});

    EXPECT_EQ(
        result.location(),
        "/login?error=missing_credentials"
    );
    EXPECT_TRUE(result.cookies().empty());
}

TEST(PortalAuthTests, LoginRejectsInvalidCredentials) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto result =
        app.post<PortalLoginAction>({
            {"username", "admin"},
            {"password", "wrong"}
        });

    EXPECT_EQ(
        result.location(),
        "/login?error=invalid_credentials"
    );
    EXPECT_TRUE(result.cookies().empty());
}

TEST(PortalAuthTests, LoginCreatesAuthenticatedSession) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto result =
        app.post<PortalLoginAction>({
            {"username", "admin"},
            {"password", "admin"}
        });

    EXPECT_EQ(result.location(), "/dashboard");
    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].name, "session_id");
    EXPECT_FALSE(result.cookies()[0].value.empty());

    auto store =
        app.services().requireService<
            drogular::SessionStore
        >();

    const auto session =
        store->get(result.cookies()[0].value);

    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->get("username").value(), "admin");
    EXPECT_EQ(session->get("role").value(), "admin");
}

TEST(PortalAuthTests, LogoutRemovesSessionAndClearsCookie) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto sessionId =
        createSession(
            app,
            "admin",
            "admin"
        );
    auto context =
        makeActionContext(app, {}, sessionId);

    PortalLogoutAction action;
    const auto result =
        action.handle(context);

    auto store =
        app.services().requireService<drogular::SessionStore>();

    EXPECT_FALSE(store->contains(sessionId));
    EXPECT_EQ(result.location(), "/login");
    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].name, "session_id");
    EXPECT_EQ(result.cookies()[0].value, "");
}

TEST(PortalAuthTests, LogoutWithoutSessionStillRedirectsAndClearsCookie) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto result =
        app.post<
            PortalLogoutAction
        >();

    EXPECT_EQ(result.location(), "/login");
    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].name, "session_id");
    EXPECT_EQ(result.cookies()[0].value, "");
}

TEST(PortalAuthTests, RenderContextResolvesCurrentUserFromSession) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto sessionId =
        createSession(
            app,
            "admin",
            "admin"
        );
    auto context =
        makeRenderContext(app, sessionId);

    const auto user =
        PortalAuthSupport::currentUser(context);

    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, "admin");
    EXPECT_EQ(user->role, "admin");
    EXPECT_TRUE(PortalAuthSupport::isAuthenticated(context));
    EXPECT_TRUE(PortalAuthSupport::hasRole(context, "admin"));
    EXPECT_FALSE(PortalAuthSupport::hasRole(context, "user"));
}

TEST(PortalAuthTests, RenderContextRejectsMissingOrUnknownSession) {
    PortalApplicationTestHost app(
        DemoDataset::create());

    auto missingContext =
        makeRenderContext(app);
    EXPECT_FALSE(
        PortalAuthSupport::currentUser(missingContext).has_value()
    );
    EXPECT_FALSE(
        PortalAuthSupport::isAuthenticated(missingContext)
    );

    auto unknownContext =
        makeRenderContext(app, "unknown-session");

    EXPECT_FALSE(
        PortalAuthSupport::currentUser(unknownContext).has_value()
    );
    EXPECT_FALSE(
        PortalAuthSupport::isAuthenticated(unknownContext)
    );
}

TEST(PortalAuthTests, ActionContextResolvesUserThroughProvider) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto sessionId =
        createSession(
            app,
            "user",
            "user"
        );
    auto context =
        makeActionContext(app, {}, sessionId);

    const auto user =
        PortalAuthSupport::currentUser(context);

    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, "user");
    EXPECT_EQ(user->role, "user");
    EXPECT_TRUE(
        PortalAuthSupport::isAuthenticated(context)
    );
    EXPECT_TRUE(
        PortalAuthSupport::hasRole(context, "user")
    );
}

TEST(PortalAuthTests, ActionContextRejectsSessionForUnknownUser) {
    PortalApplicationTestHost app(
        DemoDataset::create()
    );

    const auto sessionId =
        createSession(
            app,
            "deleted-user",
            "admin");
    auto context =
        makeActionContext(app, {}, sessionId);

    EXPECT_FALSE(
        PortalAuthSupport::currentUser(context).has_value()
    );
    EXPECT_FALSE(
        PortalAuthSupport::isAuthenticated(context)
    );
}