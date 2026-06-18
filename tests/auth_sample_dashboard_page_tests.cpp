#include "../examples/auth_sample/dashboard_page.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(AuthSampleDashboardPageTests, ShowsLoginRequiredForGuest) {
    drogular::ApplicationServices services;

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    const auto result =
        drogular::test::renderPage<DashboardPage>(
            &services
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

    const auto result =
        drogular::test::renderPage<DashboardPage>(
            &services
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