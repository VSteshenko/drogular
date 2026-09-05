#pragma once

#include <drogular/template_renderable.hpp>
#include <drogular/component.hpp>

namespace system_monitor {

class I2cFragmentComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext&) override;
    std::string templatePath() const override {
        return "fragments/i2c.html";
    }
};

} // namespace system_monitor