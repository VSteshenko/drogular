#include "services/spi_service.hpp"

#include <gtest/gtest.h>

namespace {

class SpiFakeProvider final: public system_monitor::SpiProvider {
public:
    int calls = 0;

    std::vector<system_monitor::SpiDeviceInfo> devices() override {
        ++calls;
        return {
            { 0, 0, "/dev/spidev0.0" }
        };
    }
};

} // namespace

TEST(SpiServiceTests, CachesInventory) {
    auto provider = std::make_shared<SpiFakeProvider>();
    system_monitor::SpiService service(provider, std::chrono::minutes(1));

    EXPECT_EQ(service.snapshot().devices.size(), 1U);
    EXPECT_EQ(service.snapshot().devices.size(), 1U);
    EXPECT_EQ(provider->calls, 1);
    EXPECT_EQ(service.statistics().cacheHits, 1U);
    EXPECT_TRUE(service.statistics().healthy);
}

TEST(SpiServiceTests, ReportsUnavailable) {
    system_monitor::SpiService service(nullptr);

    EXPECT_FALSE(service.available());
    EXPECT_TRUE(service.snapshot().devices.empty());
}