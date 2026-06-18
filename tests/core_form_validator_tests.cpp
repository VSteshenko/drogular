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

TEST(CoreFormValidatorTests, PassesRequiredWhenValueExists) {
    auto context =
        makeContext("title=Hello");

    const auto result =
        drogular::FormValidator(context)
            .required("title")
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(CoreFormValidatorTests, FailsRequiredWhenValueIsMissing) {
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

TEST(CoreFormValidatorTests, PassesMinLengthWhenLongEnough) {
    auto context =
        makeContext("title=Hello");

    const auto result =
        drogular::FormValidator(context)
            .minLength("title", 3)
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(CoreFormValidatorTests, FailsMinLengthWhenTooShort) {
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

TEST(CoreFormValidatorTests, ChainsRules) {
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

TEST(CoreFormValidatorTests, PassesMaxLengthWhenShortEnough) {
    auto context =
        makeContext("title=Hello");

    const auto result =
        drogular::FormValidator(context)
            .maxLength("title", 10)
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(CoreFormValidatorTests, FailsMaxLengthWhenTooLong) {
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

TEST(CoreFormValidatorTests, PassesMaxLengthWhenValueIsMissing) {
    auto context =
        makeContext("");

    const auto result =
        drogular::FormValidator(context)
            .maxLength("title", 5)
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(CoreFormValidatorTests, PassesValidEmail) {
    auto context =
        makeContext("email=test@example.com");

    const auto result =
        drogular::FormValidator(context)
            .email("email")
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(CoreFormValidatorTests, FailsEmailWithoutAt) {
    auto context =
        makeContext("email=test.example.com");

    const auto result =
        drogular::FormValidator(context)
            .email("email")
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "email");
}

TEST(CoreFormValidatorTests, FailsEmailWithoutDomainDot) {
    auto context =
        makeContext("email=test@example");

    const auto result =
        drogular::FormValidator(context)
            .email("email")
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "email");
}

TEST(CoreFormValidatorTests, PassesEmailWhenValueIsMissing) {
    auto context =
        makeContext("");

    const auto result =
        drogular::FormValidator(context)
            .email("email")
            .validate();

    EXPECT_TRUE(result.valid());
}

TEST(CoreFormValidatorTests, RequiredEmailFailsWhenMissing) {
    auto context =
        makeContext("");

    const auto result =
        drogular::FormValidator(context)
            .required("email")
            .email("email")
            .validate();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "email");
}