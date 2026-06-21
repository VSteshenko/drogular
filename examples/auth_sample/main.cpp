#include "admin_page.hpp"
#include "auth_service.hpp"
#include "home_page.hpp"
#include "dashboard_page.hpp"
#include "login_action.hpp"
#include "login_page.hpp"
#include "logout_action.hpp"

#include <drogular/app.hpp>

int main()
{
    drogular::App app;

    app.services().add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    app.services().add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    app.templateRoot("examples/auth_sample/templates")
        .templateCache(false);

    app.page<HomePage>("/");
    app.page<LoginPage>("/login");
    app.page<DashboardPage>("/dashboard");
    app.page<AdminPage>("/admin");

    app.action<LoginAction>("/login");
    app.action<LogoutAction>("/logout");

    app.run(8081);

    return 0;
}