#include <drogular/render_context.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/graphql_variables.hpp>
#include <drogular/services.hpp>
#include <drogular/translation_support.hpp>

#include <stdexcept>
#include <any>
#include <string>
#include <json/json.h>

namespace drogular {

void GraphQLResult::merge(GraphQLResult other) {
    for (auto& [key, value] : other.values_) {
        values_[std::move(key)] = std::move(value);
    }
}

bool GraphQLResult::contains(const std::string& key) const {
    return values_.contains(key);
}

void GraphQLResult::clear() {
    values_.clear();
}

Json::Value GraphQLResult::toJson() const
{
    Json::Value json(Json::objectValue);

    for (const auto& [name, value] : values_) {
        if (value.type() == typeid(std::string)) {
            json[name] = std::any_cast<std::string>(value);
        } else if (value.type() == typeid(const char*)) {
            json[name] = std::string(
                std::any_cast<const char*>(value)
            );
        } else if (value.type() == typeid(int)) {
            json[name] = std::any_cast<int>(value);
        } else if (value.type() == typeid(bool)) {
            json[name] = std::any_cast<bool>(value);
        } else if (value.type() == typeid(double)) {
            json[name] = std::any_cast<double>(value);
        } else if (value.type() == typeid(Json::Value)) {
            json[name] = std::any_cast<Json::Value>(value);
        }
    }

    return json;
}

RenderContextError::RenderContextError(const std::string& message)
    : DrogularError(message) {
}

RenderContext::RenderContext(const RenderContext* parent)
    : parent_(parent),
      serviceScope_(
          parent != nullptr
              ? parent->serviceScope_
              : std::make_shared<ServiceScope>()
      ) {
}

ApplicationServices* RenderContext::services() {
    return services_;
}

const ApplicationServices* RenderContext::services() const {
    return services_;
}

RenderContext RenderContext::createChild() const {
    RenderContext child(this);

    child.setServices(services_);
    child.setGraphQLClient(graphqlClient_);

    return child;
}

void RenderContext::setGraphQLClient(GraphQLClient* client) {
    graphqlClient_ = client;
}

bool RenderContext::hasGraphQLClient() const {
    return graphqlClient_ != nullptr;
}

void RenderContext::executeGraphQL(
    const gql::Query& query,
    const GraphQLVariables& variables
) {
    if (services_ != nullptr && services_->graphQLClient() != nullptr) {
        mergeGraphQL(
            services_->graphQLClient()->execute(
                query,
                variables
            )
            .toResult()
        );
        return;
    }

    if (graphqlClient_ != nullptr) {
        mergeGraphQL(
            graphqlClient_->execute(
                query,
                variables
            )
            .toResult()
        );
        return;
    }

    throw RenderContextError("GraphQL client is not set");
}

void RenderContext::executeGraphQL(
    const gql::Mutation& mutation,
    const GraphQLVariables& variables
) {
    if (services_ != nullptr && services_->graphQLClient() != nullptr) {
        mergeGraphQL(
            services_->graphQLClient()->execute(
                mutation,
                variables
            )
            .toResult()
        );
        return;
    }

    if (graphqlClient_ != nullptr) {
        mergeGraphQL(
            graphqlClient_->execute(
                mutation,
                variables
            )
            .toResult()
        );
        return;
    }

    throw RenderContextError("GraphQL client is not set");
}

void RenderContext::setServices(ApplicationServices* services) {
    services_ = services;
}

bool RenderContext::hasServices() const {
    return services_ != nullptr;
}

GraphQLResult& RenderContext::graphql() {
    return graphql_;
}

const GraphQLResult& RenderContext::graphql() const {
    return graphql_;
}

void RenderContext::mergeGraphQL(GraphQLResult result) {
    graphql_.merge(std::move(result));
}

void RenderContext::setRequest(
    const drogon::HttpRequestPtr& request
) {
    request_ = request;
}

drogon::HttpRequestPtr RenderContext::request() const {
    return request_;
}

std::optional<std::string> RenderContext::cookie(
    const std::string& name
) const {
    if (request_ == nullptr) {
        return std::nullopt;
    }

    const auto value =
        request_->getCookie(name);

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

std::string RenderContext::translate(
    const std::string& key
) {
    return TranslationSupport::translate(
        *this,
        key
    );
}

void RenderContext::setTranslated(
    const std::string& name,
    const std::string& translationKey
) {
    set(
        name,
        translate(translationKey)
    );
}

void RenderContext::setTranslations(
    std::initializer_list<
        std::pair<std::string, std::string>
    > values
) {
    for (const auto& [name, key] : values) {
        setTranslated(
            name,
            key
        );
    }
}

void RenderContext::setRouteParam(
    const std::string& name,
    const std::string& value
) {
    routeParams_[name] = value;
}

std::optional<std::string> RenderContext::routeParam(
    const std::string& name
) const {
    const auto found =
        routeParams_.find(name);

    if (found == routeParams_.end()) {
        return std::nullopt;
    }

    return found->second;
}

std::string RenderContext::requireRouteParam(
    const std::string& name
) const {
    const auto value =
        routeParam(name);

    if (!value.has_value()) {
        throw RenderContextError(
            "Missing route parameter: " + name
        );
    }

    return *value;
}

bool RenderContext::contains(const std::string& key) const {
    if (values_.contains(key)) {
        return true;
    }

    if (parent_ != nullptr) {
        return parent_->contains(key);
    }

    return false;
}

void RenderContext::remove(const std::string& key) {
    values_.erase(key);
}

void RenderContext::clear() {
    values_.clear();
}

} // namespace drogular