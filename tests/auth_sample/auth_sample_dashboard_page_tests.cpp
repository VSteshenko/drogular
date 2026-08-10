#include "dashboard_page.hpp"
#include "auth_sample_test_services.hpp"
#include "auth_sample_test_helpers.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(AuthSampleDashboardPageTests, ShowsLoginRequiredForGuest) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureAuthSampleTestServices(services, options);

    auto request =
        drogon::HttpRequest::newHttpRequest();

    const auto result =
        drogular::test::renderPage<DashboardPage>(
            &services,
            request
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Login required"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/login"
        )
    );
}

TEST(AuthSampleDashboardPageTests, ShowsDashboardForAuthenticatedUser) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureAuthSampleTestServices(services, options);

    auto request =
    makeAuthSampleRequestWithSession(
        services,
        AuthUser{
            .id = 1,
            .username = "admin",
            .role = "admin"
        }
    );

    const auto result =
        drogular::test::renderPage<DashboardPage>(
            &services,
            request
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Dashboard"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Welcome, admin."
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/logout"
        )
    );
}

TEST(AuthSampleDashboardPageTests, RejectsIncompleteAuthenticatedSession) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureAuthSampleTestServices(services, options);

    auto sessionStore =
        services.requireService<drogular::SessionStore>();

    auto session = sessionStore->create();
    session->set("username", "admin");

    const auto sessionId =
        session->get("_id").value();

    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->addCookie("session_id", sessionId);

    const auto result =
        drogular::test::renderPage<DashboardPage>(
            &services,
            request
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Login required"
        )
    );

    EXPECT_FALSE(
        drogular::test::contains(
            result.html,
            "Welcome, admin."
        )
    );
}