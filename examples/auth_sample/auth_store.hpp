#pragma once

#include "auth_user.hpp"

#include <drogular/state.hpp>

#include <optional>

class AuthStore {
public:
    drogular::State<
        std::optional<AuthUser>
    > currentUser;
};