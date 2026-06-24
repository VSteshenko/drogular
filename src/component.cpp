#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/template_engine.hpp>
#include <drogular/template_loader.hpp>
#include <drogular/template_preprocessor.hpp>

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

void Component::onDestroy(RenderContext&) {
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

    std::string templateSource;

    TemplateLoader loader;

    if (context.services() != nullptr &&
        context.services()->options() != nullptr) {
        loader = TemplateLoader(
            context.services()
                ->options()
                ->templateRoot()
        );
        }

    auto loadTemplateSource =
        [&](const std::string& path) -> std::string {
            if (context.services() != nullptr &&
                context.services()->options() != nullptr &&
                context.services()->options()->templateCacheEnabled()) {
                return context.services()
                    ->templateSourceCache()
                    .load(path);
                }

            return loader.load(path);
    };

    if (!templatePath().empty()) {
        templateSource =
            loadTemplateSource(templatePath());
    } else {
        templateSource =
            templateHtml();
    }

    TemplatePreprocessor preprocessor(loader);

    templateSource =
        preprocessor.process(templateSource);

    auto html = template_engine::render(
        templateSource,
        context
    );

    if (context.services() != nullptr) {
        html = component_renderer::render(
            html,
            context.services()->components(),
            context
        );
    }

    return html;
}

HtmlComponent::HtmlComponent(std::string html)
    : html_(std::move(html)) {
}

std::string HtmlComponent::render(RenderContext&) {
    return html_;
}

} // namespace drogular