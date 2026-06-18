#include "../examples/auth_sample/auth_service.hpp"

#include <gtest/gtest.h>

TEST(AuthServiceTests, AuthenticatesAdminUser) {
    AuthService service;

    const auto user =
        service.authenticate(
            "admin",
            "password"
        );

    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->id, 1);
    EXPECT_EQ(user->username, "admin");
    EXPECT_EQ(user->role, "admin");
}

TEST(AuthServiceTests, AuthenticatesRegularUser) {
    AuthService service;

    const auto user =
        service.authenticate(
            "user",
            "password"
        );

    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->id, 2);
    EXPECT_EQ(user->username, "user");
    EXPECT_EQ(user->role, "user");
}

TEST(AuthServiceTests, RejectsInvalidCredentials) {
    AuthService service;

    const auto user =
        service.authenticate(
            "admin",
            "wrong"
        );

    EXPECT_FALSE(user.has_value());
}

TEST(AuthServiceTests, ChecksUserRole) {
    AuthService service;

    const AuthUser user{
        .id = 1,
        .username = "admin",
        .role = "admin"
    };

    EXPECT_TRUE(
        service.hasRole(user, "admin")
    );

    EXPECT_FALSE(
        service.hasRole(user, "user")
    );
}
