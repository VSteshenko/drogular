#include <drogular/action_result.hpp>

#include <gtest/gtest.h>

TEST(CoreActionResultTests, CreatesEmptyResult) {
    const auto result = drogular::ActionResult::empty();

    EXPECT_EQ(result.type(), drogular::ActionResultType::Empty);
}

TEST(CoreActionResultTests, CreatesRedirectResult) {
    const auto result =
        drogular::ActionResult::redirect("/todos");

    EXPECT_EQ(result.type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(result.location(), "/todos");
}

TEST(CoreActionResultTests, CreatesHtmlResult) {
    const auto result =
        drogular::ActionResult::html("<h1>Hello</h1>");

    EXPECT_EQ(result.type(), drogular::ActionResultType::Html);
    EXPECT_EQ(result.body(), "<h1>Hello</h1>");
}

TEST(CoreActionResultTests, CreatesJsonResult) {
    Json::Value json;
    json["ok"] = true;

    const auto result =
        drogular::ActionResult::json(json);

    EXPECT_EQ(result.type(), drogular::ActionResultType::Json);
    EXPECT_TRUE(result.json()["ok"].asBool());
}

TEST(CoreActionResultTests, StoresCookies) {
    auto result =
        drogular::ActionResult::redirect("/");

    result.cookie(
        "session_id",
        "abc123"
    );

    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].name, "session_id");
    EXPECT_EQ(result.cookies()[0].value, "abc123");
    EXPECT_EQ(result.cookies()[0].path, "/");
    EXPECT_TRUE(result.cookies()[0].httpOnly);
}

TEST(CoreActionResultTests, CreatesFileResult) {
    const auto result =
        drogular::ActionResult::file(
            "report.pdf"
        );

    EXPECT_EQ(
        result.type(),
        drogular::ActionResultType::File
    );

    EXPECT_EQ(
        result.fileInfo().path,
        std::filesystem::path("report.pdf")
    );

    EXPECT_FALSE(
        result.fileInfo().forceDownload
    );
}

TEST(CoreActionResultTests, CreatesDownloadResult) {
    const auto result =
        drogular::ActionResult::download(
            "report.pdf",
            "sales-report.pdf"
        );

    EXPECT_EQ(
        result.type(),
        drogular::ActionResultType::File
    );

    EXPECT_TRUE(
        result.fileInfo().forceDownload
    );

    EXPECT_EQ(
        result.fileInfo().downloadName,
        "sales-report.pdf"
    );
}