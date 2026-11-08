#include <drogular/job_sse_drogon.hpp>
#include <drogular/job_events.hpp>

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

TEST(JobSseDrogonTests, RecognizesAllTerminalStatesAndEvents) {
    using drogular::JobEventKind;
    using drogular::JobStatus;
    EXPECT_FALSE(drogular::isTerminalJobStatus(JobStatus::Queued));
    EXPECT_FALSE(drogular::isTerminalJobStatus(JobStatus::Running));
    EXPECT_TRUE(drogular::isTerminalJobStatus(JobStatus::Completed));
    EXPECT_TRUE(drogular::isTerminalJobStatus(JobStatus::Failed));
    EXPECT_TRUE(drogular::isTerminalJobStatus(JobStatus::Cancelled));

    EXPECT_FALSE(drogular::isTerminalJobEvent(JobEventKind::Queued));
    EXPECT_FALSE(drogular::isTerminalJobEvent(JobEventKind::Started));
    EXPECT_FALSE(drogular::isTerminalJobEvent(JobEventKind::Progress));
    EXPECT_FALSE(drogular::isTerminalJobEvent(JobEventKind::CancellationRequested));
    EXPECT_TRUE(drogular::isTerminalJobEvent(JobEventKind::Completed));
    EXPECT_TRUE(drogular::isTerminalJobEvent(JobEventKind::Failed));
    EXPECT_TRUE(drogular::isTerminalJobEvent(JobEventKind::Cancelled));
}