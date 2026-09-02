#include "services/process_service.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace {

class FakeProcessMetricsProvider final
    : public system_monitor::SystemMetricsProvider {
public:
    std::vector<system_monitor::ProcessInfo> values;
    int calls{0};
    bool fail{false};

    system_monitor::SystemSnapshot snapshot() override { return {}; }
    std::vector<system_monitor::ProcessInfo> processes() override {
        ++calls;
        if (fail) throw std::runtime_error("processes failed");
        return values;
    }
};

}

TEST(ProcessServiceTests, CachesProcessSnapshotIndependently) {
    auto provider = std::make_shared<FakeProcessMetricsProvider>();
    provider->values.push_back({.pid = 42, .name = "worker"});
    system_monitor::ProcessService service(provider, std::chrono::seconds(30));

    EXPECT_EQ(service.snapshot().front().pid, 42);
    provider->values.front().pid = 43;
    EXPECT_EQ(service.snapshot().front().pid, 42);
    EXPECT_EQ(provider->calls, 1);
    EXPECT_EQ(service.statistics().cacheHits, 1U);
}

TEST(ProcessServiceTests, ReturnsStaleProcessesAfterRefreshFailure) {
    auto provider = std::make_shared<FakeProcessMetricsProvider>();
    provider->values.push_back({.pid = 42, .name = "worker"});
    system_monitor::ProcessService service(provider, std::chrono::milliseconds::zero());
    ASSERT_EQ(service.snapshot().size(), 1U);

    provider->fail = true;
    const auto stale = service.snapshot();
    ASSERT_EQ(stale.size(), 1U);
    EXPECT_EQ(stale.front().pid, 42);
    EXPECT_FALSE(service.statistics().healthy);
    EXPECT_EQ(service.statistics().failedUpdates, 1U);
}

TEST(ProcessServiceTests, PropagatesInitialFailureAndRejectsInvalidConfiguration) {
    auto provider = std::make_shared<FakeProcessMetricsProvider>();
    provider->fail = true;
    system_monitor::ProcessService service(provider);

    EXPECT_THROW(static_cast<void>(service.snapshot()), std::runtime_error);
    EXPECT_THROW(system_monitor::ProcessService(nullptr), std::invalid_argument);
    EXPECT_THROW(system_monitor::ProcessService(provider, std::chrono::milliseconds(-1)), std::invalid_argument);
}