#include <drogular/action_validation_error.hpp>

#include <gtest/gtest.h>

TEST(CoreActionValidationErrorTests, StoresMessage) {
    drogular::ActionValidationError error("Invalid input");

    EXPECT_STREQ(
        error.what(),
        "Invalid input"
    );
}