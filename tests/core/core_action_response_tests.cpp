#include <drogular/action_response.hpp>
#include <drogular/action_validation_error.hpp>

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

TEST(CoreActionResponseTests, AppliesCookiesToEmptyResult) {
    auto result = drogular::ActionResult::empty();
    result.cookie("session_id", "abc123");

    const auto response =
        drogular::toHttpResponse(result);

    ASSERT_NE(response, nullptr);

    const auto cookies = response->cookies();
    ASSERT_TRUE(cookies.contains("session_id"));
    EXPECT_EQ(cookies.at("session_id").value(), "abc123");
}

TEST(CoreActionResponseTests, AppliesExtendedCookieOptions) {
    auto result = drogular::ActionResult::empty();
    result.cookie(
        "session_id",
        "abc123",
        drogular::CookieOptions{
            .path = "/account",
            .httpOnly = true,
            .secure = true,
            .sameSite = drogular::CookieSameSite::Strict,
            .maxAge = 3600
        }
    );

    const auto response = drogular::toHttpResponse(result);

    ASSERT_NE(response, nullptr);
    const auto cookies = response->cookies();
    ASSERT_TRUE(cookies.contains("session_id"));

    const auto& cookie = cookies.at("session_id");
    EXPECT_EQ(cookie.path(), "/account");
    EXPECT_TRUE(cookie.isHttpOnly());
    EXPECT_TRUE(cookie.isSecure());
    EXPECT_EQ(cookie.sameSite(), drogon::Cookie::SameSite::kStrict);
    ASSERT_TRUE(cookie.maxAge().has_value());
    EXPECT_EQ(*cookie.maxAge(), 3600);
}

TEST(CoreActionResponseTests, ConvertsValidationErrorToBadRequest) {
    const drogular::ActionValidationError error("Invalid form value");

    const auto response = drogular::toHttpErrorResponse(error);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->statusCode(), drogon::k400BadRequest);
    EXPECT_EQ(response->body(), "Invalid form value");
}

TEST(CoreActionResponseTests, ConvertsUnexpectedErrorToSafeInternalServerError) {
    const std::runtime_error error("database password leaked here");

    const auto response = drogular::toHttpErrorResponse(error);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->statusCode(), drogon::k500InternalServerError);
    EXPECT_EQ(response->body(), "Internal Server Error");
    EXPECT_EQ(response->body().find("password"), std::string::npos);
}