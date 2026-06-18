#include <drogular/validation_result.hpp>

#include <gtest/gtest.h>

TEST(CoreValidationResultTests, IsValidByDefault) {
    drogular::ValidationResult result;

    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result.errors().empty());
}

TEST(CoreValidationResultTests, BecomesInvalidWhenErrorIsAdded) {
    drogular::ValidationResult result;

    result.addError(
        "title",
        "Title is required"
    );

    EXPECT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].field, "title");
    EXPECT_EQ(result.errors()[0].message, "Title is required");
}

TEST(CoreValidationResultTests, StoresMultipleErrors) {
    drogular::ValidationResult result;

    result.addError("title", "Title is required");
    result.addError("email", "Email is invalid");

    EXPECT_FALSE(result.valid());
    EXPECT_EQ(result.errors().size(), 2);
}

TEST(CoreValidationResultTests, DetectsFieldErrors) {
    drogular::ValidationResult result;

    result.addError("email", "Invalid email");

    EXPECT_TRUE(result.hasError("email"));
    EXPECT_FALSE(result.hasError("title"));
}

TEST(CoreValidationResultTests, ReturnsFirstFieldError) {
    drogular::ValidationResult result;

    result.addError("email", "Invalid email");

    const auto error = result.error("email");

    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(*error, "Invalid email");
}

TEST(CoreValidationResultTests, ReturnsNulloptWhenFieldHasNoError) {
    drogular::ValidationResult result;

    EXPECT_FALSE(result.error("email").has_value());
}