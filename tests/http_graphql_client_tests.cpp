#include <drogular/graphql_client.hpp>
#include <drogular/graphql_request.hpp>
#include <drogular/graphql.hpp>

#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace {

constexpr auto TestHost = "127.0.0.1";
constexpr uint16_t TestPort = 18080;

class HttpGraphQLClientTestFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        drogon::app().registerHandler(
            "/graphql-success",
            [](
                const drogon::HttpRequestPtr&,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
            ) {
                Json::Value json;
                json["data"]["viewer"]["name"] = "Vadim";

                callback(drogon::HttpResponse::newHttpJsonResponse(json));
            },
            {drogon::Post}
        );

        drogon::app().registerHandler(
            "/graphql-error",
            [](
                const drogon::HttpRequestPtr&,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
            ) {
                Json::Value error;
                error["message"] = "GraphQL validation failed";

                Json::Value json;
                json["errors"] = Json::arrayValue;
                json["errors"].append(error);

                callback(drogon::HttpResponse::newHttpJsonResponse(json));
            },
            {drogon::Post}
        );

        drogon::app().registerHandler(
            "/graphql-variables",
            [](
                const drogon::HttpRequestPtr& request,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
            ) {
                const auto json = request->getJsonObject();

                Json::Value response;

                response["data"]["receivedId"] = (*json)["variables"]["id"].asString();

                callback(
                    drogon::HttpResponse::newHttpJsonResponse(response)
                );
            },
            {drogon::Post}
        );

        drogon::app().registerHandler(
            "/graphql-http-error",
            [](
                const drogon::HttpRequestPtr&,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
            ) {
                auto response = drogon::HttpResponse::newHttpResponse();

                response->setStatusCode(drogon::k500InternalServerError);
                response->setBody("Internal Server Error");

                callback(response);
            },
            {drogon::Post}
        );

        drogon::app().registerHandler(
            "/graphql-invalid-json",
            [](
                const drogon::HttpRequestPtr&,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
            ) {
                auto response = drogon::HttpResponse::newHttpResponse();

                response->setStatusCode(drogon::k200OK);
                response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
                response->setBody("not json");

                callback(response);
            },
            {drogon::Post}
        );

        serverThread_ = std::thread([]() {
            drogon::app()
                .addListener(TestHost, TestPort)
                .run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    static void TearDownTestSuite() {
        drogon::app().quit();

        if (serverThread_.joinable()) {
            serverThread_.join();
        }
    }

private:
    static inline std::thread serverThread_;
};

} // namespace

TEST_F(HttpGraphQLClientTestFixture, ExecutesRequestAgainstServer) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-success"
    );

    drogular::GraphQLRequest request(
        "query { viewer { name } }"
    );

    const auto response =
        client.executeRequest(request);

    ASSERT_TRUE(response.hasData());

    EXPECT_EQ(
        response.data()["viewer"]["name"].asString(),
        "Vadim"
    );
}

TEST_F(HttpGraphQLClientTestFixture, ReadsGraphQLErrorsFromServer) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-error"
    );

    drogular::GraphQLRequest request(
        "query { broken }"
    );

    const auto response =
        client.executeRequest(request);

    ASSERT_TRUE(response.hasErrors());

    const auto messages =
        response.errorMessages();

    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0], "GraphQL validation failed");
}

TEST_F(HttpGraphQLClientTestFixture, ExecuteThrowsOnGraphQLErrors) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-error"
    );

    const auto query =
        drogular::gql::query("Broken")
            .select(
                drogular::gql::field("broken")
            );

    EXPECT_THROW(
        client.execute(query),
        drogular::GraphQLClientError
    );
}

TEST_F(HttpGraphQLClientTestFixture, SendsVariablesToServer) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-variables"
    );

    drogular::GraphQLRequest request(
        "query User($id: ID!) { user(id: $id) { name } }"
    );

    request.setVariable("id", "123");

    const auto response =
        client.executeRequest(request);

    ASSERT_TRUE(response.hasData());

    EXPECT_EQ(
        response.data()["receivedId"].asString(),
        "123"
    );
}

TEST_F(HttpGraphQLClientTestFixture, ExecuteMapsResponseDataToGraphQLResult) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-success"
    );

    const auto query =
        drogular::gql::query("Viewer")
            .select(
                drogular::gql::field("viewer")
                    .children({
                        drogular::gql::field("name")
                    })
            );

    const auto result =
        client.execute(query);

    const auto viewer =
        result.require<Json::Value>("viewer");

    EXPECT_EQ(
        viewer["name"].asString(),
        "Vadim"
    );
}

TEST_F(HttpGraphQLClientTestFixture, ThrowsOnHttpErrorStatus) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-http-error"
    );

    drogular::GraphQLRequest request(
        "query { viewer { name } }"
    );

    EXPECT_THROW(
        client.executeRequest(request),
        drogular::GraphQLClientError
    );
}

TEST_F(HttpGraphQLClientTestFixture, ThrowsOnInvalidJsonResponse) {
    drogular::HttpGraphQLClient client(
        TestHost,
        TestPort,
        "/graphql-invalid-json"
    );

    drogular::GraphQLRequest request(
        "query { viewer { name } }"
    );

    EXPECT_THROW(
        client.executeRequest(request),
        drogular::GraphQLClientError
    );
}