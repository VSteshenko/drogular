#pragma once

#include "../../examples/portal_demo/data/portal_dataset.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_dataset_graphql_client.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_graphql_project_provider.hpp"
#include "../../examples/portal_demo/providers/graphql/portal_graphql_user_provider.hpp"

#include <drogular/action_context.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/services.hpp>
#include <drogular/session_store.hpp>

#include <drogon/HttpRequest.h>

#include <memory>
#include <string>
#include <unordered_map>

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
        services_.add<drogular::SessionStore>(
            drogular::ServiceLifetime::Singleton
        );

        auto userProvider =
            std::make_shared<PortalGraphQLUserProvider>(
                graphQLClient_
            );

        services_.addFactory<PortalUserProvider>(
            drogular::ServiceLifetime::Singleton,
            [userProvider] {
                return userProvider;
            }
        );

        services_.addFactory<PortalProjectProvider>(
            drogular::ServiceLifetime::Singleton,
            [client = graphQLClient_, userProvider] {
                return std::make_shared<PortalGraphQLProjectProvider>(
                    client,
                    userProvider
                );
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
            drogon::HttpRequest::newHttpRequest();

        request->setMethod(drogon::Post);

        for (const auto& [name, value] : form) {
            request->setParameter(name, value);
        }

        if (!username_.empty()) {
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

private:
    std::shared_ptr<PortalDataset> dataset_;
    std::shared_ptr<PortalDatasetGraphQLClient> graphQLClient_;
    drogular::ApplicationServices services_;

    std::string username_;
    std::string role_;
};