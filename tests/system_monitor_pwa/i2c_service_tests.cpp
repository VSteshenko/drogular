#include "services/i2c_service.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

class FakeI2cProvider : public system_monitor::I2cProvider {
public:
    std::vector<system_monitor::I2cBusInfo> busValues;
    std::vector<system_monitor::I2cDeviceInfo> deviceValues;
    int busCalls{0};
    int deviceCalls{0};
    bool throwOnBuses{false};

    std::vector<system_monitor::I2cBusInfo> buses() override {
        ++busCalls;
        if (throwOnBuses) {
            throw std::runtime_error("i2c unavailable");
        }
        return busValues;
    }

    std::optional<std::vector<system_monitor::I2cDeviceInfo>> devices(
        std::uint32_t
    ) override {
        ++deviceCalls;
        return deviceValues;
    }
};

class SlowI2cProvider final : public system_monitor::I2cProvider {
public:
    std::atomic<int> busCalls{0};
    std::atomic<int> deviceCalls{0};

    std::vector<system_monitor::I2cBusInfo> buses() override {
        ++busCalls;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        return {{.number = 1, .name = "i2c-1"}};
    }

    std::optional<std::vector<system_monitor::I2cDeviceInfo>> devices(
        std::uint32_t
    ) override {
        ++deviceCalls;
        return std::vector<system_monitor::I2cDeviceInfo>{{.address = 0x3c}};
    }
};

} // namespace

TEST(I2cServiceTests, BuildsSnapshotAndUsesFiveMinuteCache) {
    auto provider = std::make_shared<FakeI2cProvider>();
    provider->busValues = {{
        .number = 1,
        .name = "i2c-1",
        .description = "bcm2835 (i2c@7e804000)"
    }};
    provider->deviceValues = {{.address = 0x3c}};

    system_monitor::I2cService service(provider);
    const auto snapshot = service.snapshot();

    ASSERT_EQ(snapshot.buses.size(), 1U);
    EXPECT_EQ(snapshot.buses[0].bus.number, 1U);
    ASSERT_EQ(snapshot.buses[0].devices.size(), 1U);
    EXPECT_EQ(snapshot.buses[0].devices[0].address, 0x3c);
    EXPECT_TRUE(snapshot.buses[0].scanned);
    EXPECT_EQ(service.statistics().refreshInterval, std::chrono::minutes(5));
    EXPECT_TRUE(service.statistics().healthy);
}

TEST(I2cServiceTests, ReusesSnapshotWithinRefreshInterval) {
    auto provider = std::make_shared<FakeI2cProvider>();
    provider->busValues = {{.number = 1, .name = "i2c-1"}};

    system_monitor::I2cService service(provider);
    static_cast<void>(service.snapshot());
    static_cast<void>(service.snapshot());

    EXPECT_EQ(provider->busCalls, 1);
    EXPECT_EQ(provider->deviceCalls, 1);
    EXPECT_EQ(service.statistics().cacheHits, 1U);
}

TEST(I2cServiceTests, ReturnsStaleSnapshotAfterRefreshFailure) {
    auto provider = std::make_shared<FakeI2cProvider>();
    provider->busValues = {{.number = 1, .name = "i2c-1"}};

    system_monitor::I2cService service(provider, std::chrono::milliseconds::zero());
    ASSERT_EQ(service.snapshot().buses.size(), 1U);

    provider->throwOnBuses = true;
    const auto stale = service.snapshot();

    ASSERT_EQ(stale.buses.size(), 1U);
    EXPECT_FALSE(service.statistics().healthy);
    EXPECT_EQ(service.statistics().failedUpdates, 1U);
}

TEST(I2cServiceTests, PropagatesInitialFailure) {
    auto provider = std::make_shared<FakeI2cProvider>();
    provider->throwOnBuses = true;
    system_monitor::I2cService service(provider);

    EXPECT_THROW(static_cast<void>(service.snapshot()), std::runtime_error);
    EXPECT_EQ(service.statistics().failedUpdates, 1U);
}

TEST(I2cServiceTests, SharesOneRefreshAcrossConcurrentReaders) {
    auto provider = std::make_shared<SlowI2cProvider>();
    system_monitor::I2cService service(provider);

    constexpr std::size_t readerCount = 8;
    std::vector<std::thread> readers;
    for (std::size_t index = 0; index < readerCount; ++index) {
        readers.emplace_back([&service] {
            static_cast<void>(service.snapshot());
        });
    }
    for (auto& reader : readers) {
        reader.join();
    }

    EXPECT_EQ(provider->busCalls.load(), 1);
    EXPECT_EQ(provider->deviceCalls.load(), 1);
    EXPECT_EQ(service.statistics().cacheHits, readerCount - 1);
}

TEST(I2cServiceTests, SupportsUnavailablePlatform) {
    system_monitor::I2cService service(nullptr);

    EXPECT_FALSE(service.available());
    EXPECT_TRUE(service.snapshot().buses.empty());
    EXPECT_FALSE(service.statistics().available);
}

TEST(I2cServiceTests, RejectsNegativeRefreshInterval) {
    auto provider = std::make_shared<FakeI2cProvider>();
    EXPECT_THROW(
        system_monitor::I2cService(provider, std::chrono::milliseconds(-1)),
        std::invalid_argument);
}