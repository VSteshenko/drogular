#include <drogular/job_sse_drogon.hpp>

#include <drogon/drogon.h>

#include <gtest/gtest.h>

TEST(JobSseDrogonTests, RejectsMissingAuthorizationAndRuntime) {
    auto jobs = std::make_shared<drogular::JobRuntime>(1, 4);
    const auto allowed = [](const drogon::HttpRequestPtr&, drogular::JobId) {
        return true;
    };

    EXPECT_THROW(drogular::registerJobSseEndpoint(
        drogon::app(), "/test/jobs/events", jobs, {}), std::invalid_argument);
    EXPECT_THROW(drogular::registerJobSseEndpoint(
        drogon::app(), "/test/jobs/events", nullptr, allowed), std::invalid_argument);
}

TEST(JobSseDrogonTests, RejectsInvalidConnectionAndHeartbeatLimits) {
    auto jobs = std::make_shared<drogular::JobRuntime>(1, 4);
    const auto allowed = [](const drogon::HttpRequestPtr&, drogular::JobId) {
        return true;
    };
    drogular::JobSseOptions options;
    options.maxConnections = 0;

    EXPECT_THROW(drogular::registerJobSseEndpoint(
        drogon::app(), "/test/jobs/events", jobs, allowed, options), std::invalid_argument);

    options.maxConnections = 1;
    options.heartbeat = std::chrono::milliseconds::zero();
    EXPECT_THROW(drogular::registerJobSseEndpoint(
        drogon::app(), "/test/jobs/events", jobs, allowed, options), std::invalid_argument);
}