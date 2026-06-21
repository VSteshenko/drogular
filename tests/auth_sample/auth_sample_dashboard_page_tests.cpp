#include "../examples/auth_sample/dashboard_page.hpp"
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