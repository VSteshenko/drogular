#include "services/uart_service.hpp"

#include <gtest/gtest.h>

class UartFakeProvider final : public system_monitor::UartProvider {
public:
    int calls{0};
    std::vector<system_monitor::UartDeviceInfo> devices() override {
        ++calls;
        return {
            { "ttyS0", "/dev/ttyS0", {"/dev/serial0"} }
        };
    }
};

TEST(UartServiceTests, CachesInventory) {
    auto provider=std::make_shared<UartFakeProvider>();
    system_monitor::UartService service(provider,std::chrono::minutes(1));

    EXPECT_EQ(service.snapshot().devices.size(),1U);
    EXPECT_EQ(service.snapshot().devices.size(),1U);
    EXPECT_EQ(provider->calls,1);
    EXPECT_EQ(service.statistics().cacheHits,1U);
    EXPECT_TRUE(service.statistics().healthy);
}

TEST(UartServiceTests, ReportsUnavailable) {
    system_monitor::UartService service(nullptr);

    EXPECT_FALSE(service.available());
    EXPECT_TRUE(service.snapshot().devices.empty());
}