#include <drogular/component.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/template_engine.hpp>

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

RenderContextError::RenderContextError(const std::string& message)
    : std::runtime_error(message) {
}

RenderContext::RenderContext(const RenderContext* parent)
    : parent_(parent) {
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

void RenderContext::executeGraphQL(const gql::Query& query) {
    if (services_ != nullptr && services_->graphQLClient() != nullptr) {
        mergeGraphQL(services_->graphQLClient()->execute(query));
        return;
    }

    if (graphqlClient_ != nullptr) {
        mergeGraphQL(graphqlClient_->execute(query));
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

void Component::onInit(RenderContext&) {
    // Default implementation does nothing.
}

std::vector<std::shared_ptr<Component>> Component::children() {
    return {};
}

std::string Component::slot() const {
    return "";
}

void Component::applyParams(RenderContext& context) const {
    for (const auto& [key, value] : params_) {
        context.set(key, value);
    }
}

std::string TemplateComponent::render(RenderContext& context) {
    applyParams(context);

    return template_engine::render(templateHtml(), context);
}

} // namespace drogular