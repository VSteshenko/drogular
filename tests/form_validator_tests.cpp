#include <drogular/form_validator.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

drogular::ActionContext makeContext(
    const std::string& body
) {
    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setMethod(drogon::Post);
    request->setContentTypeCode(
        drogon::CT_APPLICATION_X_FORM
    );
    request->setBody(body);

    return drogular::ActionContext(
        request,
        nullptr
    );
}

} // namespace

TEST(FormValidatorTests, PassesRequiredWhenValueExists) {
    auto context =
        makeContext("title=Hello");

    const auto result =
        drogular::FormValidator(context)
            .required("title")
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(FormValidatorTests, FailsRequiredWhenValueIsMissing) {
    auto context =
        makeContext("");

    const auto result =
        drogular::FormValidator(context)
            .required("title")
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "title");
}

TEST(FormValidatorTests, PassesMinLengthWhenLongEnough) {
    auto context =
        makeContext("title=Hello");

    const auto result =
        drogular::FormValidator(context)
            .minLength("title", 3)
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(FormValidatorTests, FailsMinLengthWhenTooShort) {
    auto context =
        makeContext("title=Hi");

    const auto result =
        drogular::FormValidator(context)
            .minLength("title", 3)
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "title");
}

TEST(FormValidatorTests, ChainsRules) {
    auto context =
        makeContext("title=Hi");

    const auto result =
        drogular::FormValidator(context)
            .required("title")
            .minLength("title", 3)
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "title");
}

TEST(FormValidatorTests, PassesMaxLengthWhenShortEnough) {
    auto context =
        makeContext("title=Hello");

    const auto result =
        drogular::FormValidator(context)
            .maxLength("title", 10)
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(FormValidatorTests, FailsMaxLengthWhenTooLong) {
    auto context =
        makeContext("title=HelloWorld");

    const auto result =
        drogular::FormValidator(context)
            .maxLength("title", 5)
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "title");
}

TEST(FormValidatorTests, PassesMaxLengthWhenValueIsMissing) {
    auto context =
        makeContext("");

    const auto result =
        drogular::FormValidator(context)
            .maxLength("title", 5)
            .validate();

    EXPECT_TRUE(result.valid());
}