#pragma once

#include <drogular/page.hpp>

namespace system_monitor {

class BoardPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override;

    [[nodiscard]] std::string templatePath() const override {
        return "board.html";
    }

    [[nodiscard]] std::string layoutPath() const override {
        return "layouts/main.html";
    }
};

} // namespace system_monitor