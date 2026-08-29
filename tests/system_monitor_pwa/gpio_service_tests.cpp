#include "services/gpio_service.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

namespace {

class FakeGpioProvider final : public system_monitor::GpioProvider {
public:
    std::vector<system_monitor::GpioChipInfo> chipValues;
    std::vector<system_monitor::GpioLineInfo> lineValues;
    int chipCalls{0};
    int lineCalls{0};
    bool throwOnChips{false};

    std::vector<system_monitor::GpioChipInfo> chips() override {
        ++chipCalls;
        if (throwOnChips) {
            throw std::runtime_error("gpio discovery failed");
        }
        return chipValues;
    }

    std::vector<system_monitor::GpioLineInfo> lines(std::string_view) override {
        ++lineCalls;
        return lineValues;
    }
};

class SlowGpioProvider final : public system_monitor::GpioProvider {
public:
    std::atomic<int> chipCalls{0};
    std::atomic<int> lineCalls{0};

    std::vector<system_monitor::GpioChipInfo> chips() override {
        ++chipCalls;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return {system_monitor::GpioChipInfo{
            .name = "gpiochip0",
            .label = "test",
            .lineCount = 1
        }};
    }

    std::vector<system_monitor::GpioLineInfo> lines(std::string_view) override {
        ++lineCalls;
        return {system_monitor::GpioLineInfo{
            .offset = 0,
            .name = "GPIO0"
        }};
    }
};

} // namespace

TEST(GpioServiceTests, BuildsSnapshotFromChipAndLineProviders) {
    auto provider = std::make_shared<FakeGpioProvider>();
    provider->chipValues.push_back({
        .name = "gpiochip0",
        .label = "pinctrl-bcm2711",
        .lineCount = 58
    });
    provider->lineValues.push_back({
        .offset = 4,
        .name = "GPIO4",
        .consumer = "onewire@0",
        .direction = system_monitor::GpioLineDirection::Output,
        .drive = system_monitor::GpioLineDrive::OpenDrain,
        .activeLow = false,
        .used = true
    });

    system_monitor::GpioService service(provider);
    const auto snapshot = service.snapshot();

    ASSERT_EQ(snapshot.chips.size(), 1U);
    EXPECT_EQ(snapshot.chips[0].chip.name, "gpiochip0");
    ASSERT_EQ(snapshot.chips[0].lines.size(), 1U);
    EXPECT_EQ(snapshot.chips[0].lines[0].name, "GPIO4");
    EXPECT_EQ(snapshot.chips[0].lines[0].drive,
              system_monitor::GpioLineDrive::OpenDrain);
    EXPECT_EQ(provider->chipCalls, 1);
    EXPECT_EQ(provider->lineCalls, 1);

    const auto statistics = service.statistics();
    EXPECT_TRUE(statistics.available);
    EXPECT_TRUE(statistics.healthy);
    EXPECT_EQ(statistics.updates, 1U);
    EXPECT_EQ(statistics.refreshInterval, std::chrono::seconds(30));
}

TEST(GpioServiceTests, ReadsLinesForEveryDetectedChip) {
    auto provider = std::make_shared<FakeGpioProvider>();
    provider->chipValues = {
        {.name = "gpiochip0", .label = "main", .lineCount = 58},
        {.name = "gpiochip1", .label = "exp", .lineCount = 8}
    };

    system_monitor::GpioService service(provider);
    const auto snapshot = service.snapshot();

    EXPECT_EQ(snapshot.chips.size(), 2U);
    EXPECT_EQ(provider->chipCalls, 1);
    EXPECT_EQ(provider->lineCalls, 2);
}

TEST(GpioServiceTests, ReusesSnapshotWithinRefreshInterval) {
    auto provider = std::make_shared<FakeGpioProvider>();
    provider->chipValues.push_back({
        .name = "gpiochip0", .label = "first", .lineCount = 1
    });

    system_monitor::GpioService service(provider, std::chrono::seconds(30));
    const auto first = service.snapshot();

    provider->chipValues.front().label = "second";
    const auto second = service.snapshot();

    EXPECT_EQ(provider->chipCalls, 1);
    EXPECT_EQ(first.chips.front().chip.label, "first");
    EXPECT_EQ(second.chips.front().chip.label, "first");
    EXPECT_EQ(service.statistics().cacheHits, 1U);
}

TEST(GpioServiceTests, ReturnsStaleSnapshotWhenRefreshFails) {
    auto provider = std::make_shared<FakeGpioProvider>();
    provider->chipValues.push_back({
        .name = "gpiochip0", .label = "last-good", .lineCount = 1
    });

    system_monitor::GpioService service(
        provider,
        std::chrono::milliseconds::zero());
    ASSERT_EQ(service.snapshot().chips.front().chip.label, "last-good");

    provider->throwOnChips = true;
    const auto stale = service.snapshot();

    ASSERT_EQ(stale.chips.size(), 1U);
    EXPECT_EQ(stale.chips.front().chip.label, "last-good");
    const auto statistics = service.statistics();
    EXPECT_EQ(statistics.updates, 1U);
    EXPECT_EQ(statistics.failedUpdates, 1U);
    EXPECT_FALSE(statistics.healthy);
}

TEST(GpioServiceTests, PropagatesInitialRefreshFailure) {
    auto provider = std::make_shared<FakeGpioProvider>();
    provider->throwOnChips = true;

    system_monitor::GpioService service(provider);

    EXPECT_THROW(
        static_cast<void>(service.snapshot()),
        std::runtime_error);
    EXPECT_EQ(service.statistics().failedUpdates, 1U);
    EXPECT_FALSE(service.statistics().healthy);
}

TEST(GpioServiceTests, SharesOneRefreshAcrossConcurrentReaders) {
    auto provider = std::make_shared<SlowGpioProvider>();
    system_monitor::GpioService service(provider, std::chrono::seconds(30));

    constexpr std::size_t readerCount = 8;
    std::vector<system_monitor::GpioSnapshot> snapshots(readerCount);
    std::vector<std::thread> readers;
    readers.reserve(readerCount);

    for (std::size_t index = 0; index < readerCount; ++index) {
        readers.emplace_back([&service, &snapshots, index] {
            snapshots[index] = service.snapshot();
        });
    }
    for (auto& reader : readers) {
        reader.join();
    }

    EXPECT_EQ(provider->chipCalls.load(), 1);
    EXPECT_EQ(provider->lineCalls.load(), 1);
    EXPECT_EQ(service.statistics().cacheHits, readerCount - 1);
}

TEST(GpioServiceTests, SupportsUnavailablePlatform) {
    system_monitor::GpioService service(nullptr);

    EXPECT_FALSE(service.available());
    EXPECT_TRUE(service.snapshot().chips.empty());

    const auto statistics = service.statistics();
    EXPECT_FALSE(statistics.available);
    EXPECT_FALSE(statistics.healthy);
    EXPECT_EQ(statistics.updates, 0U);
    EXPECT_EQ(statistics.failedUpdates, 0U);
}

TEST(GpioServiceTests, RejectsNegativeRefreshInterval) {
    auto provider = std::make_shared<FakeGpioProvider>();

    EXPECT_THROW(
        system_monitor::GpioService(
            provider,
            std::chrono::milliseconds(-1)),
        std::invalid_argument);
}