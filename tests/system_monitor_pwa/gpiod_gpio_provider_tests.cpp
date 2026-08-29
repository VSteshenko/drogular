#include "gpio/gpiod_gpio_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

class FakeSystemReader final : public system_monitor::SystemReader {
public:
    system_monitor::CommandResult result;

    std::string readFile(std::string_view) override {
        throw std::runtime_error("unexpected readFile");
    }

    system_monitor::CommandResult execute(std::string_view command) override {
        if (command != "gpiodetect") {
            throw std::runtime_error("unexpected command");
        }
        return result;
    }
};

} // namespace

TEST(GpiodGpioProviderTests, ParsesDetectedGpioChips) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->result = {
        0,
        "gpiochip0 [pinctrl-bcm2711] (58 lines)\n"
        "gpiochip1 [raspberrypi-exp-gpio] (8 lines)\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto chips = provider.chips();

    ASSERT_EQ(chips.size(), 2U);
    EXPECT_EQ(chips[0].name, "gpiochip0");
    EXPECT_EQ(chips[0].label, "pinctrl-bcm2711");
    EXPECT_EQ(chips[0].lineCount, 58U);
    EXPECT_EQ(chips[1].name, "gpiochip1");
    EXPECT_EQ(chips[1].label, "raspberrypi-exp-gpio");
    EXPECT_EQ(chips[1].lineCount, 8U);
}

TEST(GpiodGpioProviderTests, IgnoresMalformedOutputLines) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->result = {
        0,
        "warning text\n"
        "gpiochip0 [test-chip] (32 lines)\n"
        "gpiochip1 malformed\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto chips = provider.chips();

    ASSERT_EQ(chips.size(), 1U);
    EXPECT_EQ(chips.front().name, "gpiochip0");
    EXPECT_EQ(chips.front().lineCount, 32U);
}

TEST(GpiodGpioProviderTests, ReportsGpiodetectFailure) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->result = {127, {}, "gpiodetect: command not found"};

    system_monitor::GpiodGpioProvider provider(reader);

    EXPECT_THROW(static_cast<void>(provider.chips()), std::runtime_error);
}

TEST(GpiodGpioProviderTests, RejectsNullReader) {
    EXPECT_THROW(
        system_monitor::GpiodGpioProvider(nullptr),
        std::invalid_argument);
}