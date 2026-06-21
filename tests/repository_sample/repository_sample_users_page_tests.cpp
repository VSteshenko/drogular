#include "../../examples/repository_sample/users_page.hpp"
#include "../../examples/repository_sample/user_repository.hpp"

#include <drogular/application_options.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

namespace {

void configureRepositorySampleTestServices(
    drogular::ApplicationServices& services,
    drogular::ApplicationOptions& options
) {
    options.setTemplateRoot(
        "../../examples/repository_sample/templates"
    );

    services.setOptions(&options);

    services.add<RepositorySampleUserRepository>(
        drogular::ServiceLifetime::Singleton
    );
}

} // namespace

TEST(RepositorySampleUsersPageTests, RendersUsers) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureRepositorySampleTestServices(
        services,
        options
    );

    const auto result =
        drogular::test::renderPage<
            RepositorySampleUsersPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Alice"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "alice@example.com"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Bob"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "bob@example.com"
        )
    );
}

TEST(RepositorySampleUsersPageTests, UsesLayoutAndPartials) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    configureRepositorySampleTestServices(
        services,
        options
    );

    const auto result =
        drogular::test::renderPage<
            RepositorySampleUsersPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Drogular Repository Sample"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "Repository Sample using DI"
        )
    );
}