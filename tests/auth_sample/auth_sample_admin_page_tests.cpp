#include "../examples/auth_sample/auth_service.hpp"
#include "../examples/auth_sample/admin_page.hpp"
#include "auth_sample_test_services.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(AuthSampleAdminPageTests, DeniesGuestAccess) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    drogular::ApplicationOptions options;

    configureAuthSampleServices(
        services,
        options
    );

    const auto result =
        drogular::test::renderPage<AdminPage>(
            &services
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Access Denied"
        )
    );
}

TEST(AuthSampleAdminPageTests, DeniesRegularUser) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<AuthStore>(
        drogular::ServiceLifetime::Singleton
    );

    auto store =
        services.requireService<AuthStore>();

    store->currentUser.set(
        AuthUser{
            .id = 2,
            .username = "user",
            .role = "user"
        }
    );

    drogular::ApplicationOptions options;

    configureAuthSampleServices(
        services,
        options
    );

    services.setOptions(&options);

    const auto result =
        drogular::test::renderPage<AdminPage>(
            &services
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Access Denied"
        )
    );
}

TEST(AuthSampleAdminPageTests, AllowsAdministrator) {
    drogular::ApplicationServices services;

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

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
        drogular::test::renderPage<AdminPage>(
            &services
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Admin Panel"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "admin"
        )
    );
}