#include "core/graphql/server/portal_graphql_server.hpp"
#include "core/graphql/server/portal_graphql_operation_support.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

class PortalGraphQLAuditLog {
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

class ContextAwareGraphQLOperations {
public:
    ContextAwareGraphQLOperations(
        std::shared_ptr<std::string> value,
        std::shared_ptr<PortalGraphQLAuditLog> auditLog
    )
        : value_(std::move(value)),
          auditLog_(std::move(auditLog)) {
    }

    void registerWith(PortalGraphQLOperationRegistry& registry) {
        registry.registerQuery(
            "ContextAwareValue",
            [this](
                const drogular::GraphQLVariables&,
                const PortalGraphQLExecutionContext& context
            ) {
                const auto actor =
                    context.value("actor").value_or("anonymous");

                auditLog_->record(actor);

                Json::Value data(Json::objectValue);
                data["contextAwareValue"] =
                    *value_ + ":" + actor;

                return PortalGraphQLOperationSupport::response(data);
            }
        );
    }

private:
    std::shared_ptr<std::string> value_;
    std::shared_ptr<PortalGraphQLAuditLog> auditLog_;
};

} // namespace

TEST(PortalGraphQLServerTests, ConstructsOperationsWithMultipleDependencies) {
    auto value =
        std::make_shared<std::string>("portal");
    auto auditLog =
        std::make_shared<PortalGraphQLAuditLog>();

    PortalGraphQLServer server;
    server.add<ContextAwareGraphQLOperations>(value, auditLog);

    const auto response =
        server.executeQuery("ContextAwareValue");
    const auto result =
        response.field("contextAwareValue");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->asString(), "portal:anonymous");
    ASSERT_EQ(auditLog->entries().size(), 1);
    EXPECT_EQ(auditLog->entries()[0], "anonymous");
}

TEST(PortalGraphQLServerTests, PassesRequestScopedExecutionContextToOperation) {
    auto value =
        std::make_shared<std::string>("portal");
    auto auditLog =
        std::make_shared<PortalGraphQLAuditLog>();

    PortalGraphQLServer server;
    server.add<ContextAwareGraphQLOperations>(value, auditLog);

    PortalGraphQLExecutionContext context;
    context.set("actor", "admin");

    const auto response = server.executeQuery(
        "ContextAwareValue",
        {},
        context
    );
    const auto result =
        response.field("contextAwareValue");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->asString(), "portal:admin");
    ASSERT_EQ(auditLog->entries().size(), 1);
    EXPECT_EQ(auditLog->entries()[0], "admin");
}