#include <drogular/app.hpp>
#include <drogular/job_client_resources.hpp>

#include <gtest/gtest.h>

#include <string_view>

TEST(JobClientResourcesTests, ShipsSharedModuleWithoutDemoLaunch) {
    const auto script = drogular::job_client_resources::script();

    EXPECT_FALSE(script.empty());
    EXPECT_NE(script.find("export class JobClient"), std::string_view::npos);
    EXPECT_NE(script.find("watch(id)"), std::string_view::npos);
    EXPECT_NE(script.find("globalThis.fetch.bind(globalThis)"),
              std::string_view::npos);
    EXPECT_EQ(script.find("createDemo"), std::string_view::npos);
    EXPECT_EQ(script.find("document."), std::string_view::npos);
    EXPECT_EQ(drogular::job_client_resources::ScriptPath,
              "/__drogular/assets/job-client.js");
}

TEST(JobClientResourcesTests, RegistrationCanBeRepeated) {
    drogular::App app;
    EXPECT_NO_THROW(app.jobClient().jobClient());
}