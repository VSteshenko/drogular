#include "actions/gpio_status_action.hpp"
#include "services/gpio_service.hpp"
#include "services/system_monitor.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

#include <memory>
#include <string_view>

namespace {

class GpioActionFakeProvider final : public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chips() override {
        return {
            {
                .name = "gpiochip0",
                .label = "pinctrl-bcm2711",
                .lineCount = 58
            },
            {
                .name = "gpiochip1",
                .label = "raspberrypi-exp-gpio",
                .lineCount = 8
            }
        };
    }

    std::vector<system_monitor::GpioLineInfo> lines(
        std::string_view chip
    ) override {
        if (chip == "gpiochip0") {
            return {
                {
                    .offset = 4,
                    .name = "GPIO4",
                    .consumer = "onewire@0",
                    .direction = system_monitor::GpioLineDirection::Output,
                    .drive = system_monitor::GpioLineDrive::OpenDrain,
                    .activeLow = false,
                    .used = true
                },
                {
                    .offset = 17,
                    .name = "GPIO17",
                    .direction = system_monitor::GpioLineDirection::Input
                }
            };
        }

        return {
            {
                .offset = 2,
                .name = "PWR_LED_OFF",
                .consumer = "PWR",
                .direction = system_monitor::GpioLineDirection::Output,
                .drive = system_monitor::GpioLineDrive::PushPull,
                .activeLow = true,
                .used = true
            }
        };
    }
};

class GpioActionSystemMetricsProvider final : public system_monitor::SystemMetricsProvider {
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

drogular::ActionResult runAction(
    const std::shared_ptr<system_monitor::GpioService>& service,
    bool withBoardMetadata = false
) {
    drogular::ApplicationServices services;
    services.registerService<system_monitor::GpioService>(service);
    if (withBoardMetadata) {
        services.registerService<system_monitor::SystemMonitor>(
            std::make_shared<system_monitor::SystemMonitor>(
                std::make_shared<GpioActionSystemMetricsProvider>()));
    }

    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext context(request, &services);
    system_monitor::GpioStatusAction action;
    return action.handle(context);
}

} // namespace

TEST(GpioStatusActionTests, ReturnsGpioInventoryAsJson) {
    auto provider = std::make_shared<GpioActionFakeProvider>();
    const auto result = runAction(
        std::make_shared<system_monitor::GpioService>(provider));

    ASSERT_EQ(result.type(), drogular::ActionResultType::Json);
    const auto& json = result.json();

    EXPECT_GT(json["timestamp"].asInt64(), 0);
    EXPECT_TRUE(json["available"].asBool());
    ASSERT_EQ(json["chips"].size(), 2U);

    const auto& gpio4 = json["chips"][0]["lines"][0];
    EXPECT_EQ(json["chips"][0]["name"].asString(), "gpiochip0");
    EXPECT_EQ(json["chips"][0]["lineCount"].asUInt(), 58U);
    EXPECT_EQ(gpio4["offset"].asUInt(), 4U);
    EXPECT_EQ(gpio4["name"].asString(), "GPIO4");
    EXPECT_EQ(gpio4["consumer"].asString(), "onewire@0");
    EXPECT_EQ(gpio4["direction"].asString(), "output");
    EXPECT_EQ(gpio4["drive"].asString(), "open-drain");
    EXPECT_FALSE(gpio4["activeLow"].asBool());
    EXPECT_TRUE(gpio4["used"].asBool());

    const auto& gpio17 = json["chips"][0]["lines"][1];
    EXPECT_EQ(gpio17["direction"].asString(), "input");
    EXPECT_EQ(gpio17["drive"].asString(), "push-pull");
    EXPECT_FALSE(gpio17["used"].asBool());

    const auto& powerLed = json["chips"][1]["lines"][0];
    EXPECT_TRUE(powerLed["activeLow"].asBool());
    EXPECT_EQ(powerLed["consumer"].asString(), "PWR");

    ASSERT_TRUE(json["monitor"].isObject());
    EXPECT_EQ(json["monitor"]["updates"].asUInt64(), 1U);
    EXPECT_EQ(json["monitor"]["failedUpdates"].asUInt64(), 0U);
    EXPECT_EQ(json["monitor"]["refreshIntervalMs"].asInt64(), 30000);
    EXPECT_TRUE(json["monitor"]["healthy"].asBool());
    EXPECT_GT(json["monitor"]["lastSuccessfulUpdate"].asInt64(), 0);
}

TEST(GpioStatusActionTests, EnrichesLinesWithBoardExposureAndHeaderPins) {
    const auto result = runAction(
        std::make_shared<system_monitor::GpioService>(
            std::make_shared<GpioActionFakeProvider>()),
        true);

    const auto& json = result.json();
    EXPECT_EQ(json["chips"][0]["exposure"].asString(), "header");
    EXPECT_EQ(json["chips"][0]["lines"][0]["exposure"].asString(), "header");
    EXPECT_EQ(json["chips"][0]["lines"][0]["physicalHeaderPin"].asUInt(), 7U);
    EXPECT_EQ(json["chips"][0]["lines"][1]["physicalHeaderPin"].asUInt(), 11U);
    EXPECT_EQ(json["chips"][1]["exposure"].asString(), "onboard");
    EXPECT_EQ(json["chips"][1]["lines"][0]["exposure"].asString(), "onboard");
    EXPECT_TRUE(json["chips"][1]["lines"][0]["physicalHeaderPin"].isNull());
}

TEST(GpioStatusActionTests, ReportsUnavailablePlatformWithoutFailure) {
    const auto result = runAction(
        std::make_shared<system_monitor::GpioService>(nullptr));

    ASSERT_EQ(result.type(), drogular::ActionResultType::Json);
    const auto& json = result.json();

    EXPECT_FALSE(json["available"].asBool());
    EXPECT_TRUE(json["chips"].isArray());
    EXPECT_TRUE(json["chips"].empty());
    EXPECT_FALSE(json["monitor"]["healthy"].asBool());
    EXPECT_EQ(json["monitor"]["updates"].asUInt64(), 0U);
}