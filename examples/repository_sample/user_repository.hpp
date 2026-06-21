#pragma once

#include "user.hpp"

#include <string>
#include <utility>
#include <vector>

class RepositorySampleUserRepository {
public:
    RepositorySampleUserRepository()
        : users_({
              {1, "Alice", "alice@example.com"},
              {2, "Bob", "bob@example.com"}
          }),
          nextId_(3)
    {
    }

    std::vector<RepositorySampleUser> all() const
    {
        return users_;
    }

    void create(
        std::string name,
        std::string email
    )
    {
        users_.push_back({
            .id = nextId_++,
            .name = std::move(name),
            .email = std::move(email)
        });
    }

private:
    std::vector<RepositorySampleUser> users_;
    int nextId_ = 1;
};