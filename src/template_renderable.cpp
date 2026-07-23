#include <drogular/template_renderable.hpp>

#include <drogular/component_renderer.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>
#include <drogular/template_loader.hpp>
#include <drogular/template_preprocessor.hpp>

namespace drogular {

std::string TemplateRenderable::renderTemplate(
    RenderContext& context
) const {
    TemplateLoader loader;

    if (context.services() != nullptr &&
        context.services()->options() != nullptr) {
        loader = TemplateLoader(
            context.services()->options()->templateRoot()
        );
    }

    const auto loadTemplateSource =
        [&](const std::string& path) -> std::string {
            if (context.services() != nullptr &&
                context.services()->options() != nullptr &&
                context.services()
                    ->options()
                    ->templateCacheEnabled()) {
                return context.services()
                    ->templateSourceCache()
                    .load(path);
            }

            return loader.load(path);
        };

    auto templateSource = templatePath().empty()
        ? templateHtml()
        : loadTemplateSource(templatePath());

    if (!layoutPath().empty()) {
        auto layoutSource = loadTemplateSource(layoutPath());
        constexpr std::string_view marker = "@content";
        const auto position = layoutSource.find(marker);

        if (position != std::string::npos) {
            layoutSource.replace(
                position,
                marker.size(),
                templateSource
            );
        }

        templateSource = std::move(layoutSource);
    }

    TemplatePreprocessor preprocessor(loader);
    templateSource = preprocessor.process(templateSource);

    const auto compiled =
        templateCache_.getOrCompile(templateSource);

    auto html = compiled->render(context);

    if (context.services() != nullptr) {
        html = component_renderer::render(
            html,
            context.services()->components(),
            context
        );
    }

    return html;
}

} // namespace drogular