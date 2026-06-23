#include "create_user_action.hpp"
#include "user_repository.hpp"
#include "users_page.hpp"

#include <drogular/app.hpp>

int main()
{
    drogular::App app;

    app.staticFiles(
        "/assets",
        "examples/auth_sample/public"
        )
        .staticFileCacheProfile(
           drogular::StaticFileCacheProfile::Development
        );

    app.templateRoot(
        "examples/repository_sample/templates"
    );

    app.templateCache(false);

    app.services().add<RepositorySampleUserRepository>(
        drogular::ServiceLifetime::Singleton
    );

    app.page<RepositorySampleUsersPage>("/users");

    app.action<RepositorySampleCreateUserAction>("/users/create");

    app.run(8081);

    return 0;
}