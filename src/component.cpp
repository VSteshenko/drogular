#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

namespace drogular {

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
    return renderTemplate(context);
}

HtmlComponent::HtmlComponent(std::string html)
    : html_(std::move(html)) {
}

std::string HtmlComponent::render(RenderContext&) {
    return html_;
}

} // namespace drogular