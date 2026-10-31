#pragma once

#include "data/mini_store_store.hpp"
#include "support/mini_store_expression_functions.hpp"

#include <drogular/action_context.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/application_options.hpp>
#include <drogular/file_translation_provider.hpp>
#include <drogular/page.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>
#include <drogular/session_store.hpp>
#include <drogular/translation_provider.hpp>

#include <drogon/HttpRequest.h>

#include <filesystem>
#include <string>
#include <type_traits>
#include <unordered_map>

class MiniStoreApplicationTestHost {
public:
    MiniStoreApplicationTestHost() {
        options_.setTemplateRoot(
            std::filesystem::path(DROGULAR_SOURCE_DIR) /
            "examples/mini_store/templates"
        );
        options_.setTemplateCacheEnabled(false);

        services_.setOptions(&options_);
        services_.expressionFunctions().registerFunction(
            "t",
            miniStoreTranslationExpressionFunction()
        );
        services_.addFactory<drogular::TranslationProvider>(
            drogular::ServiceLifetime::Singleton,
            [] {
                return std::make_shared<drogular::FileTranslationProvider>(
                    std::filesystem::path(DROGULAR_SOURCE_DIR) /
                        "examples/mini_store/resources/localization",
                    "en"
                );
            }
        );
        services_.add<MiniStoreStore>(drogular::ServiceLifetime::Singleton);
        services_.add<drogular::SessionStore>(drogular::ServiceLifetime::Singleton);
    }

    std::shared_ptr<MiniStoreStore> store() {
        return services_.requireService<MiniStoreStore>();
    }

    void setLocale(std::string locale) {
        locale_ = std::move(locale);
    }

    void clearLocale() {
        locale_.clear();
    }

    template <typename TPage>
    std::string render(
        const std::unordered_map<std::string, std::string>& query = {},
        const std::unordered_map<std::string, std::string>& routeParams = {},
        const std::string& requestPath = "/"
    ) {
        static_assert(std::is_base_of_v<drogular::Page, TPage>);

        auto request = createRequest(drogon::Get);
        request->setPath(requestPath);
        for (const auto& [name, value] : query) {
            request->setParameter(name, value);
        }

        drogular::RenderContext context;
        context.setServices(&services_);
        context.setRequest(request);
        for (const auto& [name, value] : routeParams) {
            context.setRouteParam(name, value);
        }

        TPage page;
        page.onInit(context);
        return page.render(context);
    }

    template <typename TAction>
    drogular::ActionResult post(
        const std::unordered_map<std::string, std::string>& form = {},
        const std::unordered_map<std::string, std::string>& routeParams = {}
    ) {
        return execute<TAction>(form, routeParams, false);
    }

    template <typename TAction>
    drogular::ActionResult postInteraction(
        const std::unordered_map<std::string, std::string>& form = {},
        const std::unordered_map<std::string, std::string>& routeParams = {}
    ) {
        return execute<TAction>(form, routeParams, true);
    }

    void adoptSession(const drogular::ActionResult& result) {
        for (const auto& cookie : result.cookies()) {
            if (cookie.name == "session_id") {
                sessionId_ = cookie.value;
            }
        }
    }

private:
    drogular::ApplicationServices services_;
    drogular::ApplicationOptions options_;
    std::string sessionId_;
    std::string locale_;

    drogon::HttpRequestPtr createRequest(drogon::HttpMethod method) const {
        auto request = drogon::HttpRequest::newHttpRequest();
        request->setMethod(method);
        if (!sessionId_.empty()) {
            request->addCookie("session_id", sessionId_);
        }
        if (!locale_.empty()) {
            request->addCookie("lang", locale_);
        }
        return request;
    }

    template <typename TAction>
    drogular::ActionResult execute(
        const std::unordered_map<std::string, std::string>& form,
        const std::unordered_map<std::string, std::string>& routeParams,
        bool interaction
    ) {
        auto request = createRequest(drogon::Post);
        if (interaction) {
            request->addHeader("X-Drogular-Interaction", "true");
        }
        for (const auto& [name, value] : form) {
            request->setParameter(name, value);
        }

        drogular::ActionContext context(request, &services_);
        for (const auto& [name, value] : routeParams) {
            context.setRouteParam(name, value);
        }

        TAction action;
        return action.handle(context);
    }
};