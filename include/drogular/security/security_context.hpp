#pragma once

#include <drogular/security/principal.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace drogular::security {

// A lightweight, HTTP-independent snapshot of the identity associated with
// an operation. It carries no credentials and does not grant permissions.
// The public factory does not authenticate the supplied identity: trusted
// authentication adapters must validate credentials before creating a context.
class SecurityContext final {
public:
    SecurityContext() noexcept = default;

    [[nodiscard]] static SecurityContext anonymous() noexcept {
        return {};
    }

    // The Principal is owned by a shared immutable snapshot, so a copied
    // context remains valid after the originating request is destroyed.
    [[nodiscard]] static SecurityContext forPrincipal(
        Principal principal,
        std::optional<std::string> authenticationMethod = std::nullopt) {
        return SecurityContext{
            std::make_shared<const Principal>(std::move(principal)),
            std::move(authenticationMethod)
        };
    }

    [[nodiscard]] bool authenticated() const noexcept {
        return static_cast<bool>(principal_);
    }

    [[nodiscard]] std::shared_ptr<const Principal> principal() const noexcept {
        return principal_;
    }

    [[nodiscard]] std::optional<std::string_view> authenticationMethod() const noexcept {
        if (!principal_ || !authenticationMethod_) {
            return std::nullopt;
        }
        return std::string_view{*authenticationMethod_};
    }

private:
    SecurityContext(std::shared_ptr<const Principal> principal,
                    std::optional<std::string> authenticationMethod)
        : principal_(std::move(principal)),
          authenticationMethod_(std::move(authenticationMethod)) {}

    std::shared_ptr<const Principal> principal_;
    std::optional<std::string> authenticationMethod_;
};

} // namespace drogular::security