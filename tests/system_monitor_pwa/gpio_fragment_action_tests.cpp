#include "actions/gpio_fragment_action.hpp"
#include "localization/system_monitor_translations.hpp"
#include "services/gpio_service.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>
#include <drogular/translation_provider.hpp>
#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string_view>

namespace {

class GpioFragmentProvider final : public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chips() override {
        return {{.name = "gpiochip0", .label = "pinctrl-test", .lineCount = 2}};
    }

    std::vector<system_monitor::GpioLineInfo> lines(std::string_view) override {
        return {
            {.offset = 4, .name = "GPIO4", .consumer = "onewire@0",
             .function = "GPIO", .direction = system_monitor::GpioLineDirection::Output,
             .drive = system_monitor::GpioLineDrive::OpenDrain, .used = true},
            {.offset = 17, .name = "GPIO17", .function = "GPIO",
             .direction = system_monitor::GpioLineDirection::Input}
        };
    }
};

void configureServices(drogular::ApplicationServices& services,
                       drogular::ApplicationOptions& options,
                       std::shared_ptr<system_monitor::GpioService> gpioService) {
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);
    services.expressionFunctions().registerFunction(
        "t", system_monitor::systemMonitorTranslationExpressionFunction());
    services.registerService<drogular::TranslationProvider>(
        std::make_shared<system_monitor::SystemMonitorTranslations>());
    services.registerService<system_monitor::GpioService>(std::move(gpioService));
}

std::string render(std::string filter = {}, bool german = false) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    configureServices(
        services, options,
        std::make_shared<system_monitor::GpioService>(
            std::make_shared<GpioFragmentProvider>()));
    auto request = drogon::HttpRequest::newHttpRequest();
    if (!filter.empty()) request->setParameter("filter", filter);
    if (german) request->addCookie("lang", "de");
    drogular::ActionContext context(request, &services);
    system_monitor::GpioFragmentAction action;
    return action.handle(context).body();
}

} // namespace

TEST(GpioFragmentActionTests, DefaultsToActiveLinesAndRendersServerSideRows) {
    const auto html = render();

    EXPECT_NE(html.find("GPIO4"), std::string::npos);
    EXPECT_EQ(html.find("GPIO17"), std::string::npos);
    EXPECT_NE(html.find("onewire@0"), std::string::npos);
    EXPECT_NE(html.find("gpio-used"), std::string::npos);
    EXPECT_NE(html.find("data-dg-preserve-key=\"gpio-chip:gpiochip0\""), std::string::npos);
}

TEST(GpioFragmentActionTests, AppliesFreeFilterOnServer) {
    const auto html = render("free");

    EXPECT_EQ(html.find("GPIO4"), std::string::npos);
    EXPECT_NE(html.find("GPIO17"), std::string::npos);
    EXPECT_NE(html.find("value=\"free\" checked"), std::string::npos);
}

TEST(GpioFragmentActionTests, UsesLanguageCookieForFragment) {
    const auto html = render("free", true);
    EXPECT_NE(html.find("Frei"), std::string::npos);
    EXPECT_NE(html.find("Richtung"), std::string::npos);
}

TEST(GpioFragmentActionTests, MarksUnavailableServiceForDeclarativeRuntime) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    configureServices(
        services, options,
        std::make_shared<system_monitor::GpioService>(nullptr));
    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext context(request, &services);
    system_monitor::GpioFragmentAction action;

    const auto html = action.handle(context).body();
    EXPECT_NE(html.find("data-dg-unavailable"), std::string::npos);
}