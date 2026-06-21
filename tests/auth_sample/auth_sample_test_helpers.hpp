#pragma once

#include "../examples/auth_sample/auth_service.hpp"
#include "../examples/auth_sample/auth_user.hpp"

#include <drogular/application_options.hpp>
#include <drogular/services.hpp>
#include <drogular/session_store.hpp>

#include <drogon/HttpRequest.h>

inline void configureAuthSampleTestServices(
    drogular::ApplicationServices& services,
    drogular::ApplicationOptions& options
) {
    options.setTemplateRoot(
        "../../examples/auth_sample/templates"
    );

    services.setOptions(&options);

    services.add<AuthService>(
        drogular::ServiceLifetime::Singleton
    );

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );
}

inline drogon::HttpRequestPtr makeAuthSampleRequestWithSession(
    drogular::ApplicationServices& services,
    const AuthUser& user
) {
    auto sessionStore =
        services.requireService<drogular::SessionStore>();

    auto session =
        sessionStore->create();

    session->set("user_id", std::to_string(user.id));
    session->set("username", user.username);
    session->set("role", user.role);

    const auto sessionId =
        session->get("_id").value();

    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->addCookie("session_id", sessionId);

    return request;
}
