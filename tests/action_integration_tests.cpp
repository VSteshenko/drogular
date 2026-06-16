#include <drogular/action_handler.hpp>
#include <drogular/app.hpp>

#include <drogon/drogon.h>
#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <thread>

namespace {

constexpr auto TestHost = "127.0.0.1";
constexpr uint16_t TestPort = 18081;

class EchoFormAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        Json::Value json;

        json["title"] =
            context.formValue("title").value_or("");

        return drogular::ActionResult::json(json);
    }
};

class RedirectAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext&
    ) override {
        return drogular::ActionResult::redirect("/");
    }
};

class ActionIntegrationTestFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        app_ = std::make_unique<drogular::App>();

        app_->action<EchoFormAction>("/test-actions/echo");
        app_->action<RedirectAction>("/test-actions/redirect");

        serverThread_ = std::thread([]() {
            app_->run(TestPort);
        });

        std::this_thread::sleep_for(
            std::chrono::milliseconds(300)
        );
    }

    static void TearDownTestSuite() {
        drogon::app().quit();

        if (serverThread_.joinable()) {
            serverThread_.join();
        }

        app_.reset();
    }

    static inline std::unique_ptr<drogular::App> app_;
    static inline std::thread serverThread_;
};

drogon::HttpResponsePtr postForm(
    const std::string& path,
    const std::string& body
) {
    auto client =
        drogon::HttpClient::newHttpClient(
            std::string("http://") +
            TestHost +
            ":" +
            std::to_string(TestPort)
        );

    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setMethod(drogon::Post);
    request->setPath(path);
    request->setContentTypeCode(
        drogon::CT_APPLICATION_X_FORM
    );
    request->setBody(body);

    std::promise<drogon::HttpResponsePtr> promise;
    auto future = promise.get_future();

    client->sendRequest(
        request,
        [&promise](
            drogon::ReqResult result,
            const drogon::HttpResponsePtr& response
        ) {
            if (result != drogon::ReqResult::Ok ||
                response == nullptr) {
                promise.set_value(nullptr);
                return;
            }

            promise.set_value(response);
        }
    );

    return future.get();
}

} // namespace

TEST_F(ActionIntegrationTestFixture, ReadsFormValueFromPostRequest) {
    const auto response =
        postForm(
            "/test-actions/echo",
            "title=Learn%20Drogular"
        );

    ASSERT_NE(response, nullptr);

    const auto json =
        response->getJsonObject();

    ASSERT_NE(json, nullptr);

    EXPECT_EQ(
        (*json)["title"].asString(),
        "Learn Drogular"
    );
}

TEST_F(ActionIntegrationTestFixture, ConvertsRedirectResultToHttpResponse) {
    const auto response =
        postForm(
            "/test-actions/redirect",
            ""
        );

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->statusCode(),
        drogon::k302Found
    );
}