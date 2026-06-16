#include <drogular/action_validation_error.hpp>

#include <gtest/gtest.h>

TEST(ActionValidationErrorTests, StoresMessage) {
    drogular::ActionValidationError error("Invalid input");

    EXPECT_STREQ(
        error.what(),
        "Invalid input"
    );
}
