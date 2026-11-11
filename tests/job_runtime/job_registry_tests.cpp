#include <drogular/job_registry.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

TEST(JobRegistryTests, ValidatesDefinitionsAndParametersBeforeSubmit) {
    drogular::JobRuntime runtime(1, 8);
    drogular::JobRegistry registry(runtime);
    drogular::JobDefinition def;
    def.id = "test.run";
    def.name = "Test run";
    def.parameters = {
        {
            "count",
            drogular::JobParameterType::Integer,
            true,
            "",
            1,
            5
        },
        {
            "fail",
            drogular::JobParameterType::Boolean,
            false,
            "false",
            std::nullopt,
            std::nullopt
        }
    };
    registry.registerJob(def, [](drogular::JobContext& ctx, const drogular::JobParameters& args) {
        ctx.setResult(std::stoi(args.at("count")));
    });

    EXPECT_THROW(registry.registerJob(def, [](auto&, const auto&) {}), std::invalid_argument);
    EXPECT_EQ(registry.list().size(), 1u);
    EXPECT_TRUE(registry.find("test.run").has_value());
    EXPECT_FALSE(registry.find("missing").has_value());
    EXPECT_THROW(registry.launch("missing"), std::invalid_argument);
    EXPECT_THROW(registry.launch("test.run"), std::invalid_argument);
    EXPECT_THROW(registry.launch("test.run", {{"count", "0"}}), std::invalid_argument);
    EXPECT_THROW(registry.launch("test.run", {{"count", "6"}}), std::invalid_argument);
    EXPECT_THROW(registry.launch("test.run", {{"count", "abc"}}), std::invalid_argument);
    EXPECT_THROW(registry.launch("test.run", {{"count", "2"}, {"unknown", "1"}}), std::invalid_argument);
    EXPECT_THROW(registry.launch("test.run", {{"count", "2"}, {"fail", "yes"}}), std::invalid_argument);
    const auto first = registry.launch("test.run", {{"count", "2"}});
    const auto second = registry.launch("test.run", {{"count", "3"}});
    EXPECT_EQ(second, first + 1);
    for (int attempt = 0; attempt < 200; ++attempt) {
        const auto a = runtime.find(first);
        const auto b = runtime.find(second);
        if (a && b && a->status == drogular::JobStatus::Completed &&
            b->status == drogular::JobStatus::Completed
        ) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    const auto result = runtime.find(first);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, drogular::JobStatus::Completed);
    ASSERT_NE(result->resultAs<int>(), nullptr);
    EXPECT_EQ(*result->resultAs<int>(), 2);
    runtime.shutdown();
}

TEST(JobRegistryTests, RejectsInvalidDefinitions) {
    drogular::JobRuntime runtime(1, 4);
    drogular::JobRegistry registry(runtime);
    drogular::JobDefinition def;
    def.id = "not valid";
    def.name = "Name";
    EXPECT_THROW(registry.registerJob(def, [](auto&, const auto&) {}), std::invalid_argument);
    def.id = "valid";
    def.parameters = {{"n", drogular::JobParameterType::Integer, false, "7", 1, 5}};
    EXPECT_THROW(registry.registerJob(def, [](auto&, const auto&) {}), std::invalid_argument);
}

TEST(JobRegistryTests, RestrictsDiscreteParameterChoices) {
    drogular::JobRuntime runtime(1, 4);
    drogular::JobRegistry registry(runtime);
    drogular::JobDefinition def;
    def.id = "test.memory";
    def.name = "Memory task";
    def.parameters.push_back({
        "memoryKiB",
        drogular::JobParameterType::Integer,
        false,
        "512",
        0,
        2048,
        {"0", "512", "1024", "2048"}
    });
    registry.registerJob(def, [](auto&, const auto&) {});
    EXPECT_THROW(registry.launch("test.memory", {{"memoryKiB", "1023"}}),
                 std::invalid_argument);
    EXPECT_NO_THROW(registry.launch("test.memory", {{"memoryKiB", "1024"}}));
    runtime.shutdown();
}