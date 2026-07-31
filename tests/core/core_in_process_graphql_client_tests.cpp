#include <drogular/in_process_graphql_client.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace {

class TestGraphQLOperations {
public:
    void registerWith(drogular::GraphQLOperationRegistry& registry) {
        registry.registerQuery(
            "Viewer",
            [](const drogular::GraphQLVariables& variables,
               const drogular::GraphQLExecutionContext& context) {
                Json::Value response(Json::objectValue);
                response["data"]["viewer"]["id"] =
                    variables.json()["id"];
                response["data"]["viewer"]["requestId"] =
                    context.value("requestId").value_or("");
                return drogular::GraphQLResponse(std::move(response));
            }
        );

        registry.registerMutation(
            "RenameUser",
            [](const drogular::GraphQLVariables& variables,
               const drogular::GraphQLExecutionContext& context) {
                Json::Value response(Json::objectValue);
                response["data"]["renameUser"]["name"] =
                    variables.json()["name"];
                response["data"]["renameUser"]["requestId"] =
                    context.value("requestId").value_or("");
                return drogular::GraphQLResponse(std::move(response));
            }
        );
    }
};

std::shared_ptr<drogular::GraphQLServer> createServer() {
    auto server =
        std::make_shared<drogular::GraphQLServer>();
    server->add<TestGraphQLOperations>();
    return server;
}

} // namespace

TEST(CoreInProcessGraphQLClientTests, RejectsNullServer) {
    EXPECT_THROW(
        drogular::InProcessGraphQLClient(nullptr),
        std::invalid_argument
    );
}

TEST(CoreInProcessGraphQLClientTests, ExecutesNamedQuery) {
    drogular::InProcessGraphQLClient client(createServer());

    auto query = drogular::gql::query("Viewer");
    drogular::GraphQLVariables variables;
    variables.set("id", 42);

    const auto response = client.execute(query, variables);

    ASSERT_TRUE(response.field("viewer").has_value());
    EXPECT_EQ((*response.field("viewer"))["id"].asInt(), 42);
}

TEST(CoreInProcessGraphQLClientTests, ExecutesNamedMutation) {
    drogular::InProcessGraphQLClient client(createServer());

    auto mutation = drogular::gql::mutation("RenameUser");
    drogular::GraphQLVariables variables;
    variables.set("name", "Ada");

    const auto response = client.execute(mutation, variables);

    ASSERT_TRUE(response.field("renameUser").has_value());
    EXPECT_EQ(
        (*response.field("renameUser"))["name"].asString(),
        "Ada"
    );
}

TEST(CoreInProcessGraphQLClientTests, ParsesRawQueryRequest) {
    drogular::InProcessGraphQLClient client(createServer());
    drogular::GraphQLRequest request(
        "# generated request\n"
        "query Viewer($id: Int!) { viewer(id: $id) { id } }"
    );
    request.variable("id", 7);

    const auto response = client.executeRequest(request);

    ASSERT_TRUE(response.field("viewer").has_value());
    EXPECT_EQ((*response.field("viewer"))["id"].asInt(), 7);
}

TEST(CoreInProcessGraphQLClientTests, ParsesRawMutationRequest) {
    drogular::InProcessGraphQLClient client(createServer());
    drogular::GraphQLRequest request(
        "mutation RenameUser($name: String!) "
        "{ renameUser(name: $name) { name } }"
    );
    request.variable("name", "Grace");

    const auto response = client.executeRequest(request);

    ASSERT_TRUE(response.field("renameUser").has_value());
    EXPECT_EQ(
        (*response.field("renameUser"))["name"].asString(),
        "Grace"
    );
}

TEST(CoreInProcessGraphQLClientTests, CreatesContextForEveryExecution) {
    auto invocation = 0;
    drogular::InProcessGraphQLClient client(
        createServer(),
        [&invocation] {
            ++invocation;
            drogular::GraphQLExecutionContext context;
            context.set("requestId", std::to_string(invocation));
            return context;
        }
    );

    const auto first = client.execute(
        drogular::gql::query("Viewer")
    );
    const auto second = client.execute(
        drogular::gql::query("Viewer")
    );

    EXPECT_EQ(
        (*first.field("viewer"))["requestId"].asString(),
        "1"
    );
    EXPECT_EQ(
        (*second.field("viewer"))["requestId"].asString(),
        "2"
    );
}

TEST(CoreInProcessGraphQLClientTests, RejectsAnonymousRawOperation) {
    drogular::InProcessGraphQLClient client(createServer());
    drogular::GraphQLRequest request("{ viewer { id } }");

    EXPECT_THROW(
        client.executeRequest(request),
        drogular::GraphQLClientError
    );
}

TEST(CoreInProcessGraphQLClientTests, RejectsUnsupportedSubscription) {
    drogular::InProcessGraphQLClient client(createServer());
    drogular::GraphQLRequest request(
        "subscription Updates { updates { id } }"
    );

    EXPECT_THROW(
        client.executeRequest(request),
        drogular::GraphQLClientError
    );
}