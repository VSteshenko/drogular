#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>
#include <drogular/template_engine.hpp>
#include <drogular/template_loader.hpp>
#include <drogular/template_preprocessor.hpp>

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