#include "platform/raspberry_pi_probe.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {

class FakeSystemReader final : public system_monitor::SystemReader {
public:
    std::unordered_map<std::string, std::string> files;
    std::unordered_map<std::string, system_monitor::CommandResult> commands;

    std::string readFile(std::string_view path) override {
        const auto it = files.find(std::string(path));
        if (it == files.end()) {
            throw std::runtime_error("missing fake file");
        }
        return it->second;
    }

    system_monitor::CommandResult execute(std::string_view command) override {
        const auto it = commands.find(std::string(command));
        if (it == commands.end()) {
            throw std::runtime_error("missing fake command");
        }
        return it->second;
    }
};

std::shared_ptr<FakeSystemReader> makeRaspberryPiReader() {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->files["/proc/cpuinfo"] =
        "processor : 0\n"
        "processor : 1\n"
        "processor : 2\n"
        "processor : 3\n"
        "Model : Raspberry Pi 4 Model B Rev 1.4\n"
        "Revision : c03114\n"
        "Serial : 10000000a5bd2dd2\n";
    reader->commands["cat /sys/class/thermal/thermal_zone0/temp"] =
        {0, "38946\n", {}};
    return reader;
}

} // namespace

TEST(RaspberryPiProbeTests, ReadsRaspberryPiBoardInformation) {
    auto reader = makeRaspberryPiReader();
    system_monitor::RaspberryPiProbe probe(reader);

    const auto info = probe.read();

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->model, "Raspberry Pi 4 Model B Rev 1.4");
    EXPECT_EQ(info->revision, "c03114");
    EXPECT_EQ(info->serial, "10000000a5bd2dd2");
    ASSERT_TRUE(info->temperatureCelsius.has_value());
    EXPECT_DOUBLE_EQ(*info->temperatureCelsius, 38.946);
}

TEST(RaspberryPiProbeTests, ReturnsNulloptForGenericLinuxHost) {
    auto reader = std::make_shared<FakeSystemReader>();
    reader->files["/proc/cpuinfo"] =
        "processor : 0\n"
        "model name : Test CPU\n";

    system_monitor::RaspberryPiProbe probe(reader);

    EXPECT_FALSE(probe.read().has_value());
}

TEST(RaspberryPiProbeTests, KeepsTemperatureOptionalWhenThermalZoneIsUnavailable) {
    auto reader = makeRaspberryPiReader();
    reader->commands["cat /sys/class/thermal/thermal_zone0/temp"] =
        {1, {}, "not available"};

    system_monitor::RaspberryPiProbe probe(reader);
    const auto info = probe.read();

    ASSERT_TRUE(info.has_value());
    EXPECT_FALSE(info->temperatureCelsius.has_value());
}

TEST(RaspberryPiProbeTests, IgnoresMalformedTemperature) {
    auto reader = makeRaspberryPiReader();
    reader->commands["cat /sys/class/thermal/thermal_zone0/temp"] =
        {0, "not-a-temperature\n", {}};

    system_monitor::RaspberryPiProbe probe(reader);
    const auto info = probe.read();

    ASSERT_TRUE(info.has_value());
    EXPECT_FALSE(info->temperatureCelsius.has_value());
}

TEST(RaspberryPiProbeTests, RejectsNullReader) {
    EXPECT_THROW(
        system_monitor::RaspberryPiProbe(nullptr),
        std::invalid_argument);
}