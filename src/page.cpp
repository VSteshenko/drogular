#include <drogular/page.hpp>
#include <drogular/render_context.hpp>
#include <drogular/component_renderer.hpp>
#include "drogular/template_loader.hpp"
#include <drogular/template_preprocessor.hpp>

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

std::string TemplatePage::render(RenderContext& context) {
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
        [&](const std::string& path) {
            if (context.services() != nullptr &&
                context.services()->options() != nullptr &&
                context.services()->options()->templateCacheEnabled()) {
                    return context.services()
                        ->templateSourceCache()
                        .load(path);
                }

            return loader.load(path);
        };

    std::string templateSource;

    if (!templatePath().empty()) {
        templateSource =
            loadTemplateSource(templatePath());
    } else {
        templateSource =
            templateHtml();
    }

    if (!layoutPath().empty()) {
        auto layoutSource =
            loadTemplateSource(layoutPath());

        const auto marker =
            std::string("@content");

        const auto position =
            layoutSource.find(marker);

        if (position != std::string::npos) {
            layoutSource.replace(
                position,
                marker.size(),
                templateSource
            );
        }

        templateSource =
            std::move(layoutSource);
    }

    TemplatePreprocessor preprocessor(loader);

    templateSource =
        preprocessor.process(templateSource);

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