#include "actions/i2c_status_action.hpp"
#include "services/gpio_service.hpp"
#include "services/i2c_service.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

#include <memory>
#include <optional>

namespace {

class I2cActionFakeProvider final : public system_monitor::I2cProvider {
public:
    std::vector<system_monitor::I2cBusInfo> buses() override {
        return {{
            .number = 1,
            .name = "i2c-1",
            .type = "unknown",
            .description = "bcm2835 (i2c@7e804000)",
            .algorithm = "N/A"
        }};
    }

    std::optional<std::vector<system_monitor::I2cDeviceInfo>> devices(
        std::uint32_t
    ) override {
        return std::vector<system_monitor::I2cDeviceInfo>{
            {.address = 0x3c, .claimedByKernel = false},
            {.address = 0x50, .claimedByKernel = true}
        };
    }
};

class I2cActionFakeGpioProvider final : public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chips() override {
        return {{.name = "gpiochip0", .label = "pinctrl-bcm2711", .lineCount = 58}};
    }

    std::vector<system_monitor::GpioLineInfo> lines(
        std::string_view
    ) override {
        return {
            {
                .offset = 2,
                .name = "GPIO2",
                .function = "SDA1",
                .alternateFunction = true
            },
            {
                .offset = 3,
                .name = "GPIO3",
                .function = "SCL1",
                .alternateFunction = true
            }
        };
    }
};

drogular::ActionResult runAction(
    const std::shared_ptr<system_monitor::I2cService>& service,
    const std::shared_ptr<system_monitor::GpioService>& gpioService = nullptr
) {
    drogular::ApplicationServices services;
    services.registerService<system_monitor::I2cService>(service);
    if (gpioService) {
        services.registerService<system_monitor::GpioService>(gpioService);
    }

    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext context(request, &services);
    system_monitor::I2cStatusAction action;
    return action.handle(context);
}

} // namespace

TEST(I2cStatusActionTests, ReturnsI2cInventoryAsJson) {
    const auto result = runAction(std::make_shared<system_monitor::I2cService>(
        std::make_shared<I2cActionFakeProvider>()));

    ASSERT_EQ(result.type(), drogular::ActionResultType::Json);
    const auto& json = result.json();

    EXPECT_TRUE(json["available"].asBool());
    ASSERT_EQ(json["buses"].size(), 1U);
    EXPECT_EQ(json["buses"][0]["number"].asUInt(), 1U);
    EXPECT_EQ(json["buses"][0]["name"].asString(), "i2c-1");
    EXPECT_TRUE(json["buses"][0]["scanned"].asBool());
    EXPECT_EQ(json["buses"][0]["description"].asString(),
              "bcm2835 (i2c@7e804000)");
    ASSERT_EQ(json["buses"][0]["devices"].size(), 2U);
    EXPECT_EQ(json["buses"][0]["devices"][0]["address"].asUInt(), 0x3cU);
    EXPECT_EQ(json["buses"][0]["devices"][0]["addressHex"].asString(), "0x3c");
    EXPECT_FALSE(json["buses"][0]["devices"][0]["claimedByKernel"].asBool());
    EXPECT_TRUE(json["buses"][0]["devices"][1]["claimedByKernel"].asBool());
    EXPECT_TRUE(json["buses"][0]["gpioPins"].empty());
    EXPECT_EQ(json["monitor"]["refreshIntervalMs"].asInt64(), 300000);
    EXPECT_TRUE(json["monitor"]["healthy"].asBool());
}

TEST(I2cStatusActionTests, ReportsUnavailablePlatform) {
    const auto result = runAction(
        std::make_shared<system_monitor::I2cService>(nullptr));

    const auto& json = result.json();
    EXPECT_FALSE(json["available"].asBool());
    EXPECT_TRUE(json["buses"].empty());
    EXPECT_FALSE(json["monitor"]["healthy"].asBool());
}

TEST(I2cStatusActionTests, EnrichesBusWithCorrelatedGpioPins) {
    const auto result = runAction(
        std::make_shared<system_monitor::I2cService>(
            std::make_shared<I2cActionFakeProvider>()),
        std::make_shared<system_monitor::GpioService>(
            std::make_shared<I2cActionFakeGpioProvider>()));

    const auto& pins = result.json()["buses"][0]["gpioPins"];
    ASSERT_EQ(pins.size(), 2U);
    EXPECT_EQ(pins[0]["role"].asString(), "sda");
    EXPECT_EQ(pins[0]["chip"].asString(), "gpiochip0");
    EXPECT_EQ(pins[0]["offset"].asUInt(), 2U);
    EXPECT_EQ(pins[0]["name"].asString(), "GPIO2");
    EXPECT_EQ(pins[0]["function"].asString(), "SDA1");
    EXPECT_EQ(pins[1]["role"].asString(), "scl");
    EXPECT_EQ(pins[1]["offset"].asUInt(), 3U);
    EXPECT_EQ(pins[1]["function"].asString(), "SCL1");
}