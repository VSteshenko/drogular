#include <drogular/health.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <stdexcept>

using namespace std::chrono_literals;

TEST(CoreHealthTests, EmptyRegistryIsReady) {
    drogular::HealthRegistry health;
    auto report = health.readiness();
    EXPECT_EQ(report.status, drogular::HealthStatus::Healthy);
    EXPECT_TRUE(report.checks.empty());
}

TEST(CoreHealthTests, AggregatesStatuses) {
    drogular::HealthRegistry health;
    health.add("database", [] {
        return drogular::HealthCheckResult{};
    });
    health.add("cache", [] {
        return drogular::HealthCheckResult{drogular::HealthStatus::Degraded, "slow"};
    });
    auto report = health.readiness();

    EXPECT_EQ(report.status, drogular::HealthStatus::Degraded);
    ASSERT_EQ(report.checks.size(), 2u);
    EXPECT_EQ(report.checks[0].name, "database");
}

TEST(CoreHealthTests, CatchesExceptions) {
    drogular::HealthRegistry health;
    health.add("db", []() -> drogular::HealthCheckResult {
        throw std::runtime_error("password secret");
    });
    auto report = health.readiness();

    ASSERT_EQ(report.checks.size(), 1u);
    EXPECT_EQ(report.status, drogular::HealthStatus::Unhealthy);
    EXPECT_EQ(report.checks[0].result.message, "check threw an exception");
}

TEST(CoreHealthTests, TimesOutAndDoesNotStartDuplicate) {
    drogular::HealthRegistry health;
    auto calls = std::make_shared<std::atomic<int>>(0);
    health.add("slow", [calls] {
        ++*calls;
        std::this_thread::sleep_for(150ms);
        return drogular::HealthCheckResult{};
    }, 10ms);

    EXPECT_EQ(health.readiness().status, drogular::HealthStatus::Unhealthy);
    EXPECT_EQ(health.readiness().checks[0].result.message, "check already running");
    EXPECT_EQ(calls->load(), 1);

    std::this_thread::sleep_for(180ms);
    EXPECT_EQ(health.readiness().checks[0].result.message, "check timed out");
}

TEST(CoreHealthTests, RejectsInvalidRegistrations) {
    drogular::HealthRegistry health;

    EXPECT_THROW(health.add("", [] {
        return drogular::HealthCheckResult{};
    }), std::invalid_argument);

    health.add("db", [] {
        return drogular::HealthCheckResult{};
    });
    EXPECT_THROW(health.add("db", [] {
        return drogular::HealthCheckResult{};
    }), std::invalid_argument);
}