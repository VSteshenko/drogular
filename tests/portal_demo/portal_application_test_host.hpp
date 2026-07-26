#pragma once

#include "data/portal_dataset.hpp"
#include "features/projects/graphql/portal_graphql_project_provider.hpp"
#include "features/projects/providers/project_provider.hpp"
#include "localization/portal_translations.hpp"
#include "providers/graphql/portal_dataset_graphql_client.hpp"
#include "providers/graphql/portal_graphql_project_type_provider.hpp"
#include "features/roles/graphql/portal_graphql_role_provider.hpp"
#include "features/users/graphql/portal_graphql_user_provider.hpp"
#include "ui/components/portal_pagination_component.hpp"
#include "ui/components/portal_select_component.hpp"

#include <drogular/application_options.hpp>
#include <drogular/page.hpp>
#include <drogular/render_context.hpp>
#include <drogular/translation_provider.hpp>
#include <drogular/action_context.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/services.hpp>
#include <drogular/session_store.hpp>

#include <drogon/HttpRequest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <type_traits>

class PortalApplicationTestHost {
public:
    explicit PortalApplicationTestHost(
        PortalDataset dataset
    )
        : dataset_(
            std::make_shared<PortalDataset>(
                std::move(dataset)
            )
        ),
        graphQLClient_(
            std::make_shared<PortalDatasetGraphQLClient>(
                dataset_
            )
        )
    {
        options_.setTemplateRoot(
            std::filesystem::path(
                DROGULAR_SOURCE_DIR
            ) / "examples/portal_demo/templates"
        );

        options_.setTemplateCacheEnabled(false);

        services_.setOptions(&options_);
        services_.components().registerComponent<PortalSelectComponent>();
        services_.components().registerComponent<PortalPaginationComponent>();

        services_.add<drogular::SessionStore>(
            drogular::ServiceLifetime::Singleton
        );

        auto userProvider =
            std::make_shared<PortalGraphQLUserProvider>(
                graphQLClient_
            );

        services_.addFactory<PortalRoleProvider>(
            drogular::ServiceLifetime::Singleton,
            [client = graphQLClient_] {
                return std::make_shared<PortalGraphQLRoleProvider>(
                    client
                );
            }
        );

        services_.addFactory<PortalUserProvider>(
            drogular::ServiceLifetime::Singleton,
            [userProvider] {
                return userProvider;
            }
        );

        services_.addFactory<PortalProjectTypeProvider>(
            drogular::ServiceLifetime::Singleton,
            [client = graphQLClient_] {
                return std::make_shared<PortalGraphQLProjectTypeProvider>(
                    client
                );
            }
        );

        services_.addFactory<PortalProjectProvider>(
            drogular::ServiceLifetime::Singleton,
            [client = graphQLClient_] {
                return std::make_shared<PortalGraphQLProjectProvider>(
                    client
                );
            }
        );

        services_.addFactory<drogular::TranslationProvider>(
            drogular::ServiceLifetime::Singleton,
            [] {
                return std::make_shared<PortalTranslations>();
            }
        );
    }

    PortalDataset& dataset() {
        return *dataset_;
    }

    const PortalDataset& dataset() const {
        return *dataset_;
    }

    void loginAs(
        const std::string& username,
        const std::string& role
    ) {
        username_ = username;
        role_ = role;
    }

    void loginAsAdmin() {
        loginAs(
            "admin",
            "admin"
        );
    }

    void loginAsUser() {
        loginAs(
            "user",
            "user"
        );
    }

    void logout() {
        username_.clear();
        role_.clear();
    }

    template <typename TAction>
    drogular::ActionResult post(
        const std::unordered_map<std::string, std::string>& form = {}
    ) {
        return execute<TAction>(
            form
        );
    }

    template <typename TAction>
    drogular::ActionResult post(
        const std::unordered_map<std::string, std::string>& form,
        const std::unordered_map<std::string, std::string>& routeParams
    ) {
        return execute<TAction>(
            form,
            routeParams
        );
    }

    std::size_t projectCount() const {
        return dataset_->projects().size();
    }

    std::size_t userCount() const {
        return dataset_->users().size();
    }

    template <typename TAction>
    drogular::ActionResult execute(
        const std::unordered_map<std::string, std::string>& form = {},
        const std::unordered_map<std::string, std::string>& routeParams = {}
    ) {
        auto request =
            createRequest(drogon::Post);

        for (const auto& [name, value] : form) {
            request->setParameter(name, value);
        }

        drogular::ActionContext context(
            request,
            &services_
        );

        for (const auto& [name, value] : routeParams) {
            context.setRouteParam(
                name,
                value
            );
        }

        TAction action;

        return action.handle(context);
    }

    template <typename TPage>
    std::string render(
        const std::unordered_map<std::string, std::string>& query = {},
        const std::unordered_map<std::string, std::string>& routeParams = {}
    ) {
        static_assert(
            std::is_base_of_v<drogular::Page, TPage>,
            "TPage must inherit from drogular::Page"
        );

        auto request =
            createRequest(drogon::Get);

        for (const auto& [name, value] : query) {
            request->setParameter(name, value);
        }

        drogular::RenderContext context;

        context.setServices(&services_);
        context.setRequest(request);

        for (const auto& [name, value] : routeParams) {
            context.setRouteParam(
                name,
                value
            );
        }

        TPage page;

        page.onInit(context);

        return page.render(context);
    }

private:
    std::shared_ptr<PortalDataset> dataset_;
    std::shared_ptr<PortalDatasetGraphQLClient> graphQLClient_;
    drogular::ApplicationServices services_;
    drogular::ApplicationOptions options_;

    std::string username_;
    std::string role_;

    drogon::HttpRequestPtr createRequest(
        drogon::HttpMethod method
    ) {
        auto request =
            drogon::HttpRequest::newHttpRequest();

        request->setMethod(method);

        if (username_.empty()) {
            return request;
        }

        drogular::ActionContext setupContext(
            request,
            &services_
        );

        auto session =
            setupContext.session();

        session->set("username", username_);
        session->set("role", role_);

        const auto sessionId =
            session->get("_id");

        if (sessionId.has_value()) {
            request->addCookie(
                "session_id",
                *sessionId
            );
        }

        return request;
    }
};