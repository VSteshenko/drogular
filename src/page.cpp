#include <drogular/page.hpp>
#include <drogular/component_renderer.hpp>
#include "drogular/template_loader.hpp"

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

std::string TemplatePage::render(RenderContext& context) {
    std::string templateSource;

    if (!templatePath().empty()) {
        if (!templatePath().empty()) {
            if (context.services() != nullptr) {
                auto* options =
                    context.services()->options();

                if (options != nullptr &&
                    options->templateCacheEnabled()) {
                    templateSource =
                        context.services()
                            ->templateSourceCache()
                            .load(templatePath());
                } else {
                    TemplateLoader loader;

                    if (options != nullptr) {
                        loader = TemplateLoader(
                            options->templateRoot()
                        );
                    }

                    templateSource =
                        loader.load(templatePath());
                }
            } else {
                TemplateLoader loader;

                templateSource =
                        loader.load(templatePath());
            }
        } else {
            templateSource =
                templateHtml();
        }
    } else {
        templateSource =
            templateHtml();
    }

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