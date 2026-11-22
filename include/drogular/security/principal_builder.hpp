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
    InvalidRole,
    InvalidClaimName,
    InvalidAttributeName,
    DuplicateClaim,
    DuplicateAttribute,
    ConflictingIdentityClaim
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

    PrincipalBuilder& claim(std::string name, ClaimValue value) & {
        claimEntries_.emplace_back(std::move(name), std::move(value));
        return *this;
    }

    PrincipalBuilder&& claim(std::string name, ClaimValue value) && {
        static_cast<void>(this->claim(std::move(name), std::move(value)));
        return std::move(*this);
    }

    PrincipalBuilder& attribute(std::string name, ClaimValue value) & {
        attributeEntries_.emplace_back(std::move(name), std::move(value));
        return *this;
    }

    PrincipalBuilder&& attribute(std::string name, ClaimValue value) && {
        static_cast<void>(this->attribute(std::move(name), std::move(value)));
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

        Claims::Storage claims;
        for (auto& [name, value] : claimEntries_) {
            if (blank(name)) {
                return Unexpected{PrincipalError{PrincipalErrorCode::InvalidClaimName,
                                                 "Claim name must not be empty"}};
            }
            if (name == "iss" || name == "sub") {
                const auto* text = std::get_if<std::string>(&value);
                const auto& expected = name == "iss" ? id_.issuer : id_.subject;
                if (!text || *text != expected) {
                    return Unexpected{PrincipalError{PrincipalErrorCode::ConflictingIdentityClaim,
                                                     "Identity claim conflicts with PrincipalId"}};
                }
            }
            if (!claims.emplace(std::move(name), std::move(value)).second) {
                return Unexpected{PrincipalError{PrincipalErrorCode::DuplicateClaim,
                                                 "Duplicate claim name"}};
            }
        }

        Attributes::Storage attributes;
        for (auto& [name, value] : attributeEntries_) {
            if (blank(name)) {
                return Unexpected{PrincipalError{PrincipalErrorCode::InvalidAttributeName,
                                                 "Attribute name must not be empty"}};
            }
            if (!attributes.emplace(std::move(name), std::move(value)).second) {
                return Unexpected{PrincipalError{PrincipalErrorCode::DuplicateAttribute,
                                                 "Duplicate attribute name"}};
            }
        }

        return Principal {
            std::move(id_), type_,
            std::move(displayName_),
            std::move(uniqueRoles),
            Claims{std::move(claims)},
            Attributes{
                std::move(attributes)
            }
        };
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
    std::vector<std::pair<std::string, ClaimValue>> claimEntries_;
    std::vector<std::pair<std::string, ClaimValue>> attributeEntries_;
};

} // namespace drogular::security