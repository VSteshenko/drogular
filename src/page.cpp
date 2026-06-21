#include <drogular/page.hpp>
#include <drogular/component_renderer.hpp>
#include "drogular/template_loader.hpp"
#include <drogular/template_preprocessor.hpp>

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

std::string TemplatePage::render(RenderContext& context) {
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

    if (!templatePath().empty()) {
        if (context.services() != nullptr &&
            context.services()->options() != nullptr &&
            context.services()->options()->templateCacheEnabled()) {
            templateSource =
                context.services()
                    ->templateSourceCache()
                    .load(templatePath());
        } else {
            templateSource =
                loader.load(templatePath());
        }
    } else {
        templateSource =
            templateHtml();
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