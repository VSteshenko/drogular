#include "actions/uart_status_action.hpp"
#include "services/gpio_service.hpp"
#include "services/uart_service.hpp"
#include "services/system_monitor.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

class UartActionProvider final : public system_monitor::UartProvider {
public:
    std::vector<system_monitor::UartDeviceInfo> devices() override {
        return {
            { "ttyS0", "/dev/ttyS0", {"/dev/serial0"} }
        };
    }
};

class UartSystemMetricsProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot snapshot;
        snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
            .model = "Raspberry Pi 4 Model B Rev 1.4"
        };
        return snapshot;
    }

    std::vector<system_monitor::ProcessInfo> processes() override {
        return {};
    }
};

class UartGpioProvider final : public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chips() override {
        return {
            { "gpiochip0","pinctrl", 58 }
        };
    }

    std::vector<system_monitor::GpioLineInfo> lines(std::string_view) override {
        return {
            { .offset = 14, .name = "GPIO14", .function = "TXD1", .alternateFunction = true },
            { .offset = 15, .name = "GPIO15", .function = "RXD1", .alternateFunction = true }
        };
    }
};

TEST(UartStatusActionTests, ReturnsDevicesAliasesAndGpioGroups) {
    drogular::ApplicationServices services;
    services.registerService<system_monitor::UartService>(
        std::make_shared<system_monitor::UartService>(std::make_shared<UartActionProvider>()));
    services.registerService<system_monitor::GpioService>(
        std::make_shared<system_monitor::GpioService>(std::make_shared<UartGpioProvider>()));
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(
            std::make_shared<UartSystemMetricsProvider>()));
    auto request=drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext context(request,&services);
    system_monitor::UartStatusAction action;

    const auto json=action.handle(context).json();
    EXPECT_TRUE(json["available"].asBool());
    ASSERT_EQ(json["devices"].size(), 1U);
    EXPECT_EQ(json["devices"][0]["path"].asString(), "/dev/ttyS0");
    EXPECT_EQ(json["devices"][0]["aliases"][0].asString(), "/dev/serial0");
    ASSERT_EQ(json["gpioGroups"].size(), 1U);
    EXPECT_EQ(json["gpioGroups"][0]["controller"].asUInt(), 1U);
    EXPECT_EQ(json["gpioGroups"][0]["pins"].size(), 2U);
    EXPECT_EQ(json["gpioGroups"][0]["exposure"].asString(), "header");
    EXPECT_EQ(json["gpioGroups"][0]["pins"][0]["physicalHeaderPin"].asUInt(), 8U);
    EXPECT_EQ(json["gpioGroups"][0]["pins"][1]["physicalHeaderPin"].asUInt(), 10U);
}