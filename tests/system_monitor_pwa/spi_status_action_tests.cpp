#include "actions/spi_status_action.hpp"
#include "services/gpio_service.hpp"
#include "services/spi_service.hpp"
#include "services/system_monitor.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

class SpiActionProvider final: public system_monitor::SpiProvider {
public:
    std::vector<system_monitor::SpiDeviceInfo> devices() override {
        return {
            { 0, 0, "/dev/spidev0.0" },
            { 0, 1, "/dev/spidev0.1" }
        };
    }
};

class SpiGpioProvider final: public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chips() override {
        return {
            { "gpiochip0", "pinctrl", 58 }
        };
    }

    std::vector<system_monitor::GpioLineInfo> lines(std::string_view) override {
        return {
            {
                .offset = 8,
                .name = "GPIO8",
                .consumer = "spi0 CS0",
                .function = "output",
                .activeLow = true,
                .used = true
            },
            {
                .offset = 10,
                .name = "GPIO10",
                .function = "SPI0_MOSI",
                .alternateFunction = true
            },
            {
                .offset = 11,
                .name = "GPIO11",
                .function = "SPI0_SCLK",
                .alternateFunction = true
            }
        };
    }
};

class SpiSystemMetricsProvider final : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override {
        system_monitor::SystemSnapshot snapshot;
        snapshot.raspberryPi = system_monitor::RaspberryPiInfo{
            .model = "Raspberry Pi 4 Model B Rev 1.4"
        };
        return snapshot;
    }

    std::vector<system_monitor::ProcessInfo> processes() override { return {}; }
};

} // namespace

TEST(SpiStatusActionTests, ReturnsGroupedInventoryWithGpio) {
    drogular::ApplicationServices services;
    auto service = std::make_shared<system_monitor::SpiService>(
        std::make_shared<SpiActionProvider>()
    );
    auto gpioService = std::make_shared<system_monitor::GpioService>(
        std::make_shared<SpiGpioProvider>()
    );

    services.registerService<system_monitor::SpiService>(service);
    services.registerService<system_monitor::GpioService>(gpioService);
    services.registerService<system_monitor::SystemMonitor>(
        std::make_shared<system_monitor::SystemMonitor>(
            std::make_shared<SpiSystemMetricsProvider>()));

    auto req = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext ctx(req, &services);

    system_monitor::SpiStatusAction action;
    auto json=action.handle(ctx).json();

    EXPECT_TRUE(json["available"].asBool());
    ASSERT_EQ(json["buses"].size(), 1U);
    EXPECT_EQ(json["buses"][0]["number"].asUInt(), 0U);
    EXPECT_EQ(json["buses"][0]["devices"].size(), 2U);
    ASSERT_EQ(json["buses"][0]["gpioPins"].size(), 3U);
    EXPECT_EQ(json["buses"][0]["gpioPins"][0]["role"].asString(), "ce0");
    EXPECT_EQ(json["buses"][0]["gpioPins"][0]["consumer"].asString(), "spi0 CS0");
    EXPECT_EQ(json["buses"][0]["exposure"].asString(), "header");
    EXPECT_EQ(json["buses"][0]["gpioPins"][0]["exposure"].asString(), "header");
    EXPECT_EQ(json["buses"][0]["gpioPins"][0]["physicalHeaderPin"].asUInt(), 24U);
    EXPECT_EQ(json["buses"][0]["gpioPins"][1]["physicalHeaderPin"].asUInt(), 19U);
    EXPECT_EQ(json["buses"][0]["gpioPins"][2]["physicalHeaderPin"].asUInt(), 23U);
}