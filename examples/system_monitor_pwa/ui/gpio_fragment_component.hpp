#pragma once

#include <drogular/template_renderable.hpp>
#include <drogular/component.hpp>

namespace system_monitor {

class GpioFragmentComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext& context) override;

    std::string templatePath() const override {
        return "fragments/gpio.html";
    }
};

} // namespace system_monitor