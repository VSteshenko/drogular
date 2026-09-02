#include "actions/process_status_action.hpp"
#include "services/process_service.hpp"

#include <drogular/action_context.hpp>
#include <drogular/services.hpp>
#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

namespace {

class ProcessActionProvider final
    : public system_monitor::SystemMetricsProvider {
public:
    system_monitor::SystemSnapshot snapshot() override { return {}; }
    std::vector<system_monitor::ProcessInfo> processes() override {
        return {{.pid = 123, .user = "alice", .name = "worker",
                 .command = "/usr/bin/worker --serve", .cpuPercent = 8.5,
                 .memoryPercent = 1.5, .residentBytes = 4096}};
    }
};

}

TEST(ProcessStatusActionTests, ReturnsProcessInventoryAndMonitorState) {
    drogular::ApplicationServices services;
    services.registerService<system_monitor::ProcessService>(
        std::make_shared<system_monitor::ProcessService>(
            std::make_shared<ProcessActionProvider>()));
    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext context(request, &services);
    system_monitor::ProcessStatusAction action;

    const auto json = action.handle(context).json();
    ASSERT_EQ(json["processes"].size(), 1U);
    EXPECT_EQ(json["processes"][0]["pid"].asInt64(), 123);
    EXPECT_EQ(json["processes"][0]["user"].asString(), "alice");
    EXPECT_EQ(json["processes"][0]["command"].asString(), "/usr/bin/worker --serve");
    EXPECT_TRUE(json["monitor"]["healthy"].asBool());
    EXPECT_EQ(json["monitor"]["refreshIntervalMs"].asInt64(), 2000);
}