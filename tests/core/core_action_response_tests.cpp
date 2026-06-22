#include <drogular/action_response.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

TEST(CoreActionResponseTests, ConvertsEmptyResult) {
    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::empty()
        );

    ASSERT_NE(response, nullptr);
}

TEST(CoreActionResponseTests, ConvertsRedirectResult) {
    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::redirect("/")
        );

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(
        response->statusCode(),
        drogon::k302Found
    );
}

TEST(CoreActionResponseTests, ConvertsHtmlResult) {
    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::html("<h1>Hello</h1>")
        );

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->body(), "<h1>Hello</h1>");
}

TEST(CoreActionResponseTests, ConvertsJsonResult) {
    Json::Value json;
    json["ok"] = true;

    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::json(json)
        );

    ASSERT_NE(response, nullptr);

    const auto responseJson =
        response->getJsonObject();

    ASSERT_NE(responseJson, nullptr);
    EXPECT_TRUE((*responseJson)["ok"].asBool());
}

TEST(CoreActionResponseTests, ConvertsCookiesToHttpResponse) {
    auto result =
        drogular::ActionResult::redirect("/");

    result.cookie(
        "session_id",
        "abc123"
    );

    const auto response =
        drogular::toHttpResponse(result);

    ASSERT_NE(response, nullptr);

    const auto cookies =
        response->cookies();

    ASSERT_TRUE(
        cookies.find("session_id") != cookies.end()
    );

    EXPECT_EQ(
        cookies.at("session_id").value(),
        "abc123"
    );
}

TEST(CoreActionResponseTests, ConvertsFileResultToHttpResponse) {
    const auto path =
        std::filesystem::temp_directory_path() /
        "drogular_action_file_response.txt";

    {
        std::ofstream file(path);
        file << "hello";
    }

    const auto result =
        drogular::ActionResult::file(path);

    const auto response =
        drogular::toHttpResponse(result);

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->contentTypeString(),
        "text/plain"
    );

    std::filesystem::remove(path);
}

TEST(CoreActionResponseTests, ConvertsDownloadResultToAttachmentResponse) {
    const auto path =
        std::filesystem::temp_directory_path() /
        "drogular_action_download_response.txt";

    {
        std::ofstream file(path);
        file << "hello";
    }

    const auto result =
        drogular::ActionResult::download(
            path,
            "download.txt"
        );

    const auto response =
        drogular::toHttpResponse(result);

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->getHeader("Content-Disposition"),
        "attachment; filename=\"download.txt\""
    );

    std::filesystem::remove(path);
}