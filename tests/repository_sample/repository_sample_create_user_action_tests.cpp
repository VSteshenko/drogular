#include "../../examples/repository_sample/create_user_action.hpp"
#include "../../examples/repository_sample/user_repository.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

drogular::ActionContext makeRepositorySampleCreateUserContext(
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

TEST(RepositorySampleCreateUserActionTests, CreatesUser) {
    drogular::ApplicationServices services;

    services.add<RepositorySampleUserRepository>(
        drogular::ServiceLifetime::Singleton
    );

    auto context =
        makeRepositorySampleCreateUserContext(
            "name=Charlie&email=charlie@example.com",
            services
        );

    RepositorySampleCreateUserAction action;

    const auto result =
        action.handle(context);

    EXPECT_EQ(result.location(), "/users");

    auto repository =
        services.requireService<RepositorySampleUserRepository>();

    const auto users =
        repository->all();

    ASSERT_EQ(users.size(), 3);
    EXPECT_EQ(users[2].name, "Charlie");
    EXPECT_EQ(users[2].email, "charlie@example.com");
}

TEST(RepositorySampleCreateUserActionTests, RejectsInvalidEmail) {
    drogular::ApplicationServices services;

    services.add<RepositorySampleUserRepository>(
        drogular::ServiceLifetime::Singleton
    );

    auto context =
        makeRepositorySampleCreateUserContext(
            "name=Charlie&email=invalid",
            services
        );

    RepositorySampleCreateUserAction action;

    action.handle(context);

    auto repository =
        services.requireService<RepositorySampleUserRepository>();

    EXPECT_EQ(repository->all().size(), 2);
}