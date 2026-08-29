#include "gpio/gpiod_gpio_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

class FakeSystemReader final : public system_monitor::SystemReader {
public:
    std::unordered_map<std::string, system_monitor::CommandResult> results;
    std::vector<std::string> commands;

    std::string readFile(std::string_view) override {
        throw std::runtime_error("unexpected readFile");
    }

    system_monitor::CommandResult execute(std::string_view command) override {
        commands.emplace_back(command);
        const auto found = results.find(std::string(command));
        if (found == results.end()) {
            if (command == "pinctrl get") {
                return {127, {}, "pinctrl: command not found"};
            }
            throw std::runtime_error("unexpected command: " + std::string(command));
        }
        return found->second;
    }
};

} // namespace

TEST(GpiodGpioProviderTests, ParsesDetectedGpioChips) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpiodetect"] = {
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

TEST(GpiodGpioProviderTests, IgnoresMalformedGpiodetectOutputLines) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpiodetect"] = {
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

TEST(GpiodGpioProviderTests, ParsesLibgpiod2LineInformation) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip1'"] = {
        0,
        "gpiochip1 - 8 lines:\n"
        " line   0: \"BT_ON\" output\n"
        " line   2: \"PWR_LED_OFF\" output active-low consumer=\"led1\"\n"
        " line   7: \"SD_OC_N\" input\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip1");

    ASSERT_EQ(lines.size(), 3U);

    EXPECT_EQ(lines[0].offset, 0U);
    EXPECT_EQ(lines[0].name, "BT_ON");
    EXPECT_EQ(lines[0].direction, system_monitor::GpioLineDirection::Output);
    EXPECT_EQ(lines[0].drive, system_monitor::GpioLineDrive::PushPull);
    EXPECT_FALSE(lines[0].activeLow);
    EXPECT_FALSE(lines[0].used);
    EXPECT_TRUE(lines[0].consumer.empty());

    EXPECT_EQ(lines[1].offset, 2U);
    EXPECT_EQ(lines[1].name, "PWR_LED_OFF");
    EXPECT_EQ(lines[1].consumer, "led1");
    EXPECT_EQ(lines[1].direction, system_monitor::GpioLineDirection::Output);
    EXPECT_TRUE(lines[1].activeLow);
    EXPECT_TRUE(lines[1].used);

    EXPECT_EQ(lines[2].offset, 7U);
    EXPECT_EQ(lines[2].direction, system_monitor::GpioLineDirection::Input);
}

TEST(GpiodGpioProviderTests, ParsesRaspberryPi4TabSeparatedOutputAndDrive) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        0,
        "gpiochip0 - 58 lines:\n"
        "\tline   2:\t\"GPIO2\"         \tinput\n"
        "\tline   4:\t\"GPIO4\"         \toutput drive=open-drain consumer=\"onewire@0\"\n"
        "\tline   7:\t\"GPIO7\"         \toutput active-low consumer=\"spi0 CS1\"\n"
        "\tline  42:\t\"STATUS_LED_G_CLK\"\toutput consumer=\"ACT\"\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 4U);

    EXPECT_EQ(lines[0].offset, 2U);
    EXPECT_EQ(lines[0].direction, system_monitor::GpioLineDirection::Input);
    EXPECT_EQ(lines[0].drive, system_monitor::GpioLineDrive::PushPull);
    EXPECT_FALSE(lines[0].used);

    EXPECT_EQ(lines[1].offset, 4U);
    EXPECT_EQ(lines[1].direction, system_monitor::GpioLineDirection::Output);
    EXPECT_EQ(lines[1].drive, system_monitor::GpioLineDrive::OpenDrain);
    EXPECT_EQ(lines[1].consumer, "onewire@0");
    EXPECT_TRUE(lines[1].used);

    EXPECT_EQ(lines[2].offset, 7U);
    EXPECT_EQ(lines[2].direction, system_monitor::GpioLineDirection::Output);
    EXPECT_EQ(lines[2].drive, system_monitor::GpioLineDrive::PushPull);
    EXPECT_TRUE(lines[2].activeLow);
    EXPECT_EQ(lines[2].consumer, "spi0 CS1");
    EXPECT_TRUE(lines[2].used);

    EXPECT_EQ(lines[3].offset, 42U);
    EXPECT_EQ(lines[3].consumer, "ACT");
    EXPECT_TRUE(lines[3].used);
}

TEST(GpiodGpioProviderTests, ParsesUnnamedAndUnquotedLibgpiod2Fields) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        0,
        "gpiochip0 - 3 lines:\n"
        " line 0: unnamed input\n"
        " line 1: \"GPIO1\" output consumer=gpioset\n"
        " line 2: \"GPIO2\" input consumer=\"button listener\"\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 3U);
    EXPECT_TRUE(lines[0].name.empty());
    EXPECT_FALSE(lines[0].used);
    EXPECT_EQ(lines[1].consumer, "gpioset");
    EXPECT_TRUE(lines[1].used);
    EXPECT_EQ(lines[2].consumer, "button listener");
    EXPECT_TRUE(lines[2].used);
}

TEST(GpiodGpioProviderTests, ParsesAdditionalDriveModes) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        0,
        "gpiochip0 - 3 lines:\n"
        " line 0: \"A\" output drive=open-source\n"
        " line 1: \"B\" output drive=push-pull\n"
        " line 2: \"C\" output drive=future-mode\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 3U);
    EXPECT_EQ(lines[0].drive, system_monitor::GpioLineDrive::OpenSource);
    EXPECT_EQ(lines[1].drive, system_monitor::GpioLineDrive::PushPull);
    EXPECT_EQ(lines[2].drive, system_monitor::GpioLineDrive::Unknown);
}

TEST(GpiodGpioProviderTests, FallsBackToLibgpiod1AndParsesLegacyFormat) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        1, {}, "gpioinfo: unrecognized option '-c'"
    };
    reader->results["gpioinfo 'gpiochip0'"] = {
        0,
        "gpiochip0 - 3 lines:\n"
        " line   0: \"ID_SDA\" unused input active-high\n"
        " line   1: \"LED\" \"led0\" output active-low [used]\n"
        " line   2: unnamed unused input active-high\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 3U);
    EXPECT_FALSE(lines[0].used);
    EXPECT_EQ(lines[0].direction, system_monitor::GpioLineDirection::Input);

    EXPECT_EQ(lines[1].consumer, "led0");
    EXPECT_EQ(lines[1].direction, system_monitor::GpioLineDirection::Output);
    EXPECT_TRUE(lines[1].activeLow);
    EXPECT_TRUE(lines[1].used);

    EXPECT_TRUE(lines[2].name.empty());
    ASSERT_EQ(reader->commands.size(), 3U);
    EXPECT_EQ(reader->commands[0], "gpioinfo -c 'gpiochip0'");
    EXPECT_EQ(reader->commands[1], "gpioinfo 'gpiochip0'");
}

TEST(GpiodGpioProviderTests, IgnoresMalformedGpioinfoLines) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        0,
        "gpiochip0 - 2 lines:\n"
        " nonsense\n"
        " line not-an-offset: \"BAD\" input\n"
        " line 4: \"GOOD\" input\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 1U);
    EXPECT_EQ(lines.front().offset, 4U);
    EXPECT_EQ(lines.front().name, "GOOD");
}

TEST(GpiodGpioProviderTests, ReportsGpiodetectFailure) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpiodetect"] = {127, {}, "gpiodetect: command not found"};

    system_monitor::GpiodGpioProvider provider(reader);

    EXPECT_THROW(static_cast<void>(provider.chips()), std::runtime_error);
}

TEST(GpiodGpioProviderTests, ReportsGpioinfoFailureAfterCompatibilityFallback) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {1, {}, "unsupported option"};
    reader->results["gpioinfo 'gpiochip0'"] = {1, {}, "permission denied"};

    system_monitor::GpiodGpioProvider provider(reader);

    EXPECT_THROW(static_cast<void>(provider.lines("gpiochip0")), std::runtime_error);
}

TEST(GpiodGpioProviderTests, QuotesChipNameBeforeExecutingShellCommand) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip'\\''0'"] = {
        0,
        "gpiochip0 - 0 lines:\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    EXPECT_TRUE(provider.lines("gpiochip'0").empty());
}

TEST(GpiodGpioProviderTests, RejectsEmptyChipName) {
    auto reader = std::make_shared<FakeSystemReader>();
    system_monitor::GpiodGpioProvider provider(reader);

    EXPECT_THROW(
        static_cast<void>(provider.lines("")),
        std::invalid_argument);
}

TEST(GpiodGpioProviderTests, RejectsNullReader) {
    EXPECT_THROW(
        system_monitor::GpiodGpioProvider(nullptr),
        std::invalid_argument);
}

TEST(GpiodGpioProviderTests, EnrichesPrimaryChipWithRaspberryPiPinFunctions) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        0,
        "gpiochip0 - 3 lines:\n"
        " line 2: \"GPIO2\" input\n"
        " line 3: \"GPIO3\" input\n"
        " line 4: \"GPIO4\" output consumer=\"onewire@0\"\n",
        {}
    };
    reader->results["pinctrl get"] = {
        0,
        " 2: a0    pu | hi // GPIO2 = SDA1\n"
        " 3: a0    pu | hi // GPIO3 = SCL1\n"
        " 4: op dh pu | hi // GPIO4 = output\n",
        {}
    };

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 3U);
    EXPECT_EQ(lines[0].function, "SDA1");
    EXPECT_TRUE(lines[0].alternateFunction);
    EXPECT_FALSE(lines[0].used);
    EXPECT_EQ(lines[1].function, "SCL1");
    EXPECT_TRUE(lines[1].alternateFunction);
    EXPECT_EQ(lines[2].function, "output");
    EXPECT_FALSE(lines[2].alternateFunction);
}

TEST(GpiodGpioProviderTests, KeepsGenericLinuxInventoryWhenPinctrlIsUnavailable) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->results["gpioinfo -c 'gpiochip0'"] = {
        0,
        "gpiochip0 - 1 lines:\n line 2: \"GPIO2\" input\n",
        {}
    };
    reader->results["pinctrl get"] = {127, {}, "pinctrl: command not found"};

    system_monitor::GpiodGpioProvider provider(reader);
    const auto lines = provider.lines("gpiochip0");

    ASSERT_EQ(lines.size(), 1U);
    EXPECT_TRUE(lines[0].function.empty());
    EXPECT_FALSE(lines[0].alternateFunction);
}