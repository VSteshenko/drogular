#include "../../examples/repository_sample/user_repository.hpp"

#include <gtest/gtest.h>

TEST(RepositorySampleUserRepositoryTests, ContainsDefaultUsers) {
    RepositorySampleUserRepository repository;

    const auto users =
        repository.all();

    ASSERT_EQ(users.size(), 2);

    EXPECT_EQ(users[0].name, "Alice");
    EXPECT_EQ(users[0].email, "alice@example.com");

    EXPECT_EQ(users[1].name, "Bob");
    EXPECT_EQ(users[1].email, "bob@example.com");
}

TEST(RepositorySampleUserRepositoryTests, CreatesUser) {
    RepositorySampleUserRepository repository;

    repository.create(
        "Charlie",
        "charlie@example.com"
    );

    const auto users =
        repository.all();

    ASSERT_EQ(users.size(), 3);

    EXPECT_EQ(users[2].id, 3);
    EXPECT_EQ(users[2].name, "Charlie");
    EXPECT_EQ(users[2].email, "charlie@example.com");
}