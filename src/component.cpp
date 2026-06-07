#include <drogular/component.hpp>

namespace drogular {

bool GraphQLResult::contains(const std::string& key) const {
    return values_.contains(key);
}

void GraphQLResult::clear() {
    values_.clear();
}

GraphQLResult& RenderContext::graphql() {
    return graphql_;
}

const GraphQLResult& RenderContext::graphql() const {
    return graphql_;
}

RenderContextError::RenderContextError(const std::string& message)
    : std::runtime_error(message) {
}

bool RenderContext::contains(const std::string& key) const {
    return values_.contains(key);
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

} // namespace drogular