#pragma once

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>
#include <drogular/page.hpp>

#include <concepts>
#include <string>

template <typename PageType>
    requires std::derived_from<PageType, drogular::TemplatePage>
class PortalPageFragmentComponent final
    : public drogular::TemplateComponent
{
public:
    std::string templatePath() const override {
        return PageType{}.templatePath();
    }

    std::string layoutPath() const override {
        return "layouts/content.html";
    }
};

template <typename PageType>
    requires std::derived_from<PageType, drogular::TemplatePage>
class PortalPageFragmentAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(context)) {
            return *result;
        }

        return drogular::ActionRenderer::render<
            PortalPageFragmentComponent<PageType>
        >(
            context,
            [](drogular::RenderContext& renderContext) {
                PageType page;
                page.onInit(renderContext);
            }
        );
    }
};