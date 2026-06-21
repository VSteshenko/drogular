#include "../examples/auth_sample/home_page.hpp"
#include "auth_sample_test_services.hpp"
#include "auth_sample_test_helpers.hpp"

#include <drogular/testing.hpp>

#include <gtest/gtest.h>

TEST(AuthSampleHomePageTests, RedirectsGuestToLogin) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureAuthSampleTestServices(services, options);

    auto request =
        drogon::HttpRequest::newHttpRequest();

    const auto result =
        drogular::test::renderPage<HomePage>(
            &services,
            request
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
        drogular::test::renderPage<HomePage>(
            &services,
            request
        );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "url=/dashboard"
        )
    );
}