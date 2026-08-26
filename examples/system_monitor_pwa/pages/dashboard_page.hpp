#pragma once

#include <drogular/page.hpp>

namespace system_monitor {

class DashboardPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override;

    [[nodiscard]] std::string templatePath() const override {
        return "dashboard.html";
    }

    [[nodiscard]] std::string layoutPath() const override {
        return "layouts/main.html";
    }
};

} // namespace system_monitor