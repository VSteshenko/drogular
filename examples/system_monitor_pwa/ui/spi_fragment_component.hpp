#pragma once

#include <drogular/template_renderable.hpp>
#include <drogular/component.hpp>

namespace system_monitor {

class SpiFragmentComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext&) override;
    std::string templatePath() const override {
        return "fragments/spi.html";
    }
};

} // namespace system_monitor