#pragma once

#include <drogular/expected.hpp>
#include <drogular/security/principal.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

namespace drogular::security {

enum class PrincipalErrorCode {
    MissingIssuer,
    MissingSubject,
    InvalidRole
};

struct PrincipalError final {
    PrincipalErrorCode code;
    std::string message;
};

class PrincipalBuilder final {
public:
    PrincipalBuilder& issuer(std::string value) & {
        id_.issuer = std::move(value);
        return *this;
    }
    PrincipalBuilder&& issuer(std::string value) && {
        static_cast<void>(this->issuer(std::move(value)));
        return std::move(*this);
    }
    PrincipalBuilder& subject(std::string value) & {
        id_.subject = std::move(value);
        return *this;
    }
    PrincipalBuilder&& subject(std::string value) && {
        static_cast<void>(this->subject(std::move(value)));
        return std::move(*this);
    }
    PrincipalBuilder& type(PrincipalType value) & {
        type_ = value;
        return *this;
    }
    PrincipalBuilder&& type(PrincipalType value) && {
        static_cast<void>(this->type(value));
        return std::move(*this);
    }
    PrincipalBuilder& displayName(std::string value) & {
        displayName_ = std::move(value);
        return *this;
    }
    PrincipalBuilder&& displayName(std::string value) && {
        static_cast<void>(this->displayName(std::move(value)));
        return std::move(*this);
    }
    PrincipalBuilder& role(std::string value) & {
        roles_.push_back(std::move(value));
        return *this;
    }
    PrincipalBuilder&& role(std::string value) && {
        static_cast<void>(this->role(std::move(value)));
        return std::move(*this);
    }

    // Building consumes the builder; validated Principal values expose no mutators.
    [[nodiscard]] Expected<Principal, PrincipalError> build() && {
        if (blank(id_.issuer)) {
            return Unexpected{PrincipalError{PrincipalErrorCode::MissingIssuer,
                                             "Principal issuer must not be empty"}};
        }
        if (blank(id_.subject)) {
            return Unexpected{PrincipalError{PrincipalErrorCode::MissingSubject,
                                             "Principal subject must not be empty"}};
        }
        std::vector<std::string> uniqueRoles;
        uniqueRoles.reserve(roles_.size());
        for (auto& role : roles_) {
            if (blank(role)) {
                return Unexpected{PrincipalError{PrincipalErrorCode::InvalidRole,
                                                 "Principal role must not be empty"}};
            }
            if (std::find(uniqueRoles.begin(), uniqueRoles.end(), role) == uniqueRoles.end()) {
                uniqueRoles.push_back(std::move(role));
            }
        }
        return Principal{std::move(id_), type_, std::move(displayName_),
                         std::move(uniqueRoles)};
    }

private:
    static bool blank(const std::string& value) {
        return std::all_of(value.begin(), value.end(), [](unsigned char ch) {
            return std::isspace(ch) != 0;
        });
    }

    PrincipalId id_;
    PrincipalType type_ = PrincipalType::User;
    std::string displayName_;
    std::vector<std::string> roles_;
};

} // namespace drogular::security