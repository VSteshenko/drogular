#include "../examples/auth_sample/home_page.hpp"
#include "auth_sample_test_services.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(AuthSampleHomePageTests, RedirectsGuestToLogin) {
    drogular::ApplicationServices services;

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    drogular::ApplicationOptions options;

    configureAuthSampleServices(
        services,
        options
    );

    services.setOptions(&options);

    const auto result =
        drogular::test::renderPage<HomePage>(
            &services
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "url=/login"
        )
    );
}

TEST(AuthSampleHomePageTests, RedirectsAuthenticatedUserToDashboard) {
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

    drogular::ApplicationOptions options;

    configureAuthSampleServices(
        services,
        options
    );

    services.setOptions(&options);

    const auto result =
        drogular::test::renderPage<HomePage>(
            &services
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "url=/dashboard"
        )
    );
}