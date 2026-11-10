#include <drogular/process_metrics.hpp>
#include <drogular/app.hpp>

#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <thread>

TEST(ProcessMetricsTests, SamplesProcessIndependentlyOfJobs) {
    drogular::ProcessMetricsProvider provider;
    const auto first = provider.snapshot();

#if defined(__linux__) || defined(__APPLE__)
    ASSERT_TRUE(first.available);
    EXPECT_GT(first.residentBytes, 0u);
    EXPECT_GE(first.peakResidentBytes, first.residentBytes);
    EXPECT_GT(first.threadCount, 0u);
    EXPECT_FALSE(first.cpuPercent.has_value());
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const auto second = provider.snapshot();
    EXPECT_GE(second.uptimeMs, first.uptimeMs);
    EXPECT_GE(second.cpuTimeMs, first.cpuTimeMs);
#if defined(__linux__) || defined(__APPLE__)
    ASSERT_TRUE(second.cpuPercent.has_value());
    EXPECT_GE(*second.cpuPercent, 0.0);
#endif
}

TEST(ProcessMetricsTests, AppInspectionExposesIndependentSection) {
    drogular::App app;
    const auto inspection = app.inspect();
    const auto section =
        std::find_if(
            inspection.sections.begin(),
            inspection.sections.end(),
            [](const auto& s) {
                return s.id == "process";
            }
        );

    ASSERT_NE(section, inspection.sections.end());
    EXPECT_EQ(section->component, "drogular.process");
    EXPECT_TRUE(section->data.isMember("available"));
    EXPECT_TRUE(section->data.isMember("threadCount"));
    EXPECT_TRUE(section->data.isMember("cpuPercent"));
}