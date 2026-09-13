#include "actions/hardware_fragment_action.hpp"
#include "localization/system_monitor_translations.hpp"
#include "services/system_monitor.hpp"
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


class HardwareGpioProvider final : public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chips() override {
        return {{.name = "gpiochip0", .label = "pinctrl-bcm2711", .lineCount = 58}};
    }

    std::vector<system_monitor::GpioLineInfo> lines(std::string_view) override {
        return {
            {.offset = 14, .name = "GPIO14", .function = "TXD0",
             .direction = system_monitor::GpioLineDirection::Output,
             .used = true, .alternateFunction = true},
            {.offset = 15, .name = "GPIO15", .function = "RXD0",
             .direction = system_monitor::GpioLineDirection::Input,
             .used = true, .alternateFunction = true}
        };
    }
};

class HardwareFragmentProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot value;
        value.system.hostname = "hardware-host";
        value.system.operatingSystem = "Linux";
        value.system.architecture = "aarch64";
        value.raspberryPi = system_monitor::RaspberryPiInfo{
            .model = "Raspberry Pi 4 Model B Rev 1.4",
            .revision = "c03114",
            .serial = "10000000a5bd2dd2"
        };
        return value;
    }

    std::vector<system_monitor::ProcessInfo> processes() override { return {}; }
};

std::string render(bool german = false) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);
    services.expressionFunctions().registerFunction(
        "t", system_monitor::systemMonitorTranslationExpressionFunction());
    services.registerService<drogular::TranslationProvider>(
        std::make_shared<system_monitor::SystemMonitorTranslations>());
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(
            std::make_shared<HardwareFragmentProvider>()));
    services.registerService<system_monitor::GpioService>(
        std::make_shared<system_monitor::GpioService>(
            std::make_shared<HardwareGpioProvider>()));

    auto request = drogon::HttpRequest::newHttpRequest();
    if (german) request->addCookie("lang", "de");
    drogular::ActionContext context(request, &services);
    system_monitor::HardwareFragmentAction action;
    return action.handle(context).body();
}

} // namespace

TEST(HardwareFragmentActionTests, RendersAggregatedHardwareOverviewServerSide) {
    const auto html = render();

    EXPECT_NE(html.find("Raspberry Pi 4 Model B Rev 1.4"), std::string::npos);
    EXPECT_NE(html.find("data-board-header-map"), std::string::npos);
    EXPECT_NE(html.find("data-physical-pin=\"1\""), std::string::npos);
    EXPECT_NE(html.find("3V3"), std::string::npos);
    EXPECT_NE(html.find("panel dg-card board-interface-card"), std::string::npos);
    EXPECT_NE(html.find("2/5 data sources reachable"), std::string::npos);
}

TEST(HardwareFragmentActionTests, UsesLanguageCookie) {
    const auto html = render(true);

    EXPECT_NE(html.find("Identität"), std::string::npos);
    EXPECT_NE(html.find("Hardware-Schnittstellen"), std::string::npos);
    EXPECT_NE(html.find("Hardware-Inventare verfügbar"), std::string::npos);
}