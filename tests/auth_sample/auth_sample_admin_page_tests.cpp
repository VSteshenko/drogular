#include "../examples/auth_sample/auth_service.hpp"
#include "../examples/auth_sample/admin_page.hpp"
#include "auth_sample_test_services.hpp"
#include "auth_sample_test_helpers.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(AuthSampleAdminPageTests, DeniesGuestAccess) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureAuthSampleTestServices(services, options);

    auto request =
        drogon::HttpRequest::newHttpRequest();

    const auto result =
        drogular::test::renderPage<AdminPage>(
            &services,
            request
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
    drogular::ApplicationOptions options;

    configureAuthSampleTestServices(services, options);

    auto request =
    makeAuthSampleRequestWithSession(
        services,
        AuthUser{
            .id = 1,
            .username = "user",
            .role = "user"
        }
    );

    const auto result =
        drogular::test::renderPage<AdminPage>(
            &services,
            request
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
        drogular::test::renderPage<AdminPage>(
            &services,
            request
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