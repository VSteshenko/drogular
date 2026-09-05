#pragma once

#include <drogular/component.hpp>

namespace system_monitor {

class SystemFragmentComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext& context) override;

    std::string templatePath() const override {
        return "fragments/system.html";
    }
};

} // namespace system_monitor