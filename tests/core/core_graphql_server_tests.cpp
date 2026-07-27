#include <drogular/graphql_server.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

drogular::GraphQLResponse responseWith(
    const std::string& field,
    const std::string& value
) {
    Json::Value data(Json::objectValue);
    data[field] = value;

    Json::Value root(Json::objectValue);
    root["data"] = std::move(data);
    return drogular::GraphQLResponse(root);
}

class AuditLog {
public:
    void record(std::string value) {
        entries_.push_back(std::move(value));
    }

    const std::vector<std::string>& entries() const {
        return entries_;
    }

private:
    std::vector<std::string> entries_;
};

class ContextAwareOperations {
public:
    ContextAwareOperations(
        std::shared_ptr<std::string> value,
        std::shared_ptr<AuditLog> auditLog
    )
        : value_(std::move(value)),
          auditLog_(std::move(auditLog)) {
    }

    void registerWith(drogular::GraphQLOperationRegistry& registry) {
        registry.registerQuery(
            "ContextAwareValue",
            [this](
                const drogular::GraphQLVariables&,
                const drogular::GraphQLExecutionContext& context
            ) {
                const auto actor =
                    context.value("actor").value_or("anonymous");

                auditLog_->record(actor);
                return responseWith(
                    "contextAwareValue",
                    *value_ + ":" + actor
                );
            }
        );
    }

private:
    std::shared_ptr<std::string> value_;
    std::shared_ptr<AuditLog> auditLog_;
};

class LegacyOperations {
public:
    void registerWith(drogular::GraphQLOperationRegistry& registry) {
        registry.registerMutation(
            "LegacyValue",
            [](const drogular::GraphQLVariables&) {
                return responseWith("legacyValue", "supported");
            }
        );
    }
};

} // namespace

TEST(GraphQLServerTests, ConstructsOperationsWithMultipleDependencies) {
    auto value =
        std::make_shared<std::string>("drogular");
    auto auditLog =
        std::make_shared<AuditLog>();

    drogular::GraphQLServer server;
    server.add<ContextAwareOperations>(value, auditLog);

    const auto response =
        server.executeQuery("ContextAwareValue");
    const auto result =
        response.field("contextAwareValue");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->asString(), "drogular:anonymous");
    ASSERT_EQ(auditLog->entries().size(), 1);
    EXPECT_EQ(auditLog->entries()[0], "anonymous");
}

TEST(GraphQLServerTests, PassesRequestScopedExecutionContextToOperation) {
    auto value =
        std::make_shared<std::string>("drogular");
    auto auditLog =
        std::make_shared<AuditLog>();

    drogular::GraphQLServer server;
    server.add<ContextAwareOperations>(value, auditLog);

    drogular::GraphQLExecutionContext context;
    context.set("actor", "admin");

    const auto response = server.executeQuery(
        "ContextAwareValue",
        {},
        context
    );
    const auto result =
        response.field("contextAwareValue");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->asString(), "drogular:admin");
    ASSERT_EQ(auditLog->entries().size(), 1);
    EXPECT_EQ(auditLog->entries()[0], "admin");
}

TEST(GraphQLServerTests, SupportsLegacyHandlersWithoutExecutionContext) {
    drogular::GraphQLServer server;
    server.add<LegacyOperations>();

    const auto response =
        server.executeMutation("LegacyValue");
    const auto result =
        response.field("legacyValue");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->asString(), "supported");
}