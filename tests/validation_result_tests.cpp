#include <drogular/validation_result.hpp>

#include <gtest/gtest.h>

TEST(ValidationResultTests, IsValidByDefault) {
    drogular::ValidationResult result;

    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result.errors().empty());
}

TEST(ValidationResultTests, BecomesInvalidWhenErrorIsAdded) {
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

TEST(ValidationResultTests, StoresMultipleErrors) {
    drogular::ValidationResult result;

    result.addError("title", "Title is required");
    result.addError("email", "Email is invalid");

    EXPECT_FALSE(result.valid());
    EXPECT_EQ(result.errors().size(), 2);
}
