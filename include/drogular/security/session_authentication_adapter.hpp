#pragma once

#include <drogular/security/authentication_adapter.hpp>
#include <drogular/security/principal_builder.hpp>
#include <drogular/session.hpp>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace drogular::security {

// A synchronous facade over a server-side Session already resolved through
// SessionStore. Never construct this from unverified client cookie contents.
// The facade must not be shared concurrently; returned views are valid until
// its destruction. No HTTP request or SessionStore ownership is required here.
class SessionAuthenticationRequest final : public AuthenticationRequest {
public:
    explicit SessionAuthenticationRequest(std::shared_ptr<Session> session)
        : session_(std::move(session))
    {
    }

    [[nodiscard]] std::optional<std::string_view> header(std::string_view) const override {
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string_view> sessionValue(
        std::string_view key) const override {
        if (!session_) {
            return std::nullopt;
        }
        const auto [it, inserted] =
            values_.try_emplace(std::string(key), std::nullopt);
        if (inserted) {
            it->second = session_->get(it->first);
        }
        if (!it->second) {
            return std::nullopt;
        }
        return std::string_view{*it->second};
    }

private:
    std::shared_ptr<Session> session_;
    mutable std::map<std::string, std::optional<std::string>> values_;
};

// Opt-in bridge for the legacy server-side session contract. It recognizes
// the same "username" key as AuthSupport::isAuthenticated(), and intentionally
// does not grant roles or permissions based on arbitrary session strings.
// This adapter does not validate a cookie or lookup a session: those operations
// belong to the HTTP integration (Stage D.4).
class SessionAuthenticationAdapter final : public AuthenticationAdapter {
public:
    explicit SessionAuthenticationAdapter(std::string issuer,
                                          std::string subjectKey = "username")
        : issuer_(std::move(issuer)), subjectKey_(std::move(subjectKey))
    {
    }

    [[nodiscard]] AuthenticationResult authenticate(
        const AuthenticationRequest& request) const override {
        if (issuer_.empty() || subjectKey_.empty()) {
            return Unexpected{AuthenticationError{AuthenticationErrorCode::InternalError}};
        }
        const auto subject = request.sessionValue(subjectKey_);
        if (!subject) {
            return std::optional<SecurityContext>{};
        }
        if (subject->empty()) {
            return Unexpected{AuthenticationError{AuthenticationErrorCode::InvalidCredentials}};
        }
        auto principal = PrincipalBuilder{}
            .issuer(issuer_)
            .subject(std::string(*subject))
            .type(PrincipalType::User)
            .build();
        if (!principal) {
            return Unexpected{AuthenticationError{AuthenticationErrorCode::InvalidCredentials}};
        }
        return std::optional<SecurityContext>{SecurityContext::forPrincipal(
            std::move(principal.value()), std::string{"session"})};
    }

private:
    std::string issuer_;
    std::string subjectKey_;
};

} // namespace drogular::security