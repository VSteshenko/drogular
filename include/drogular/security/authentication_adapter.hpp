#pragma once

#include <drogular/expected.hpp>
#include <drogular/security/security_context.hpp>

#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace drogular::security {

// Non-owning request facade. Views returned by these methods must remain valid
// for the synchronous duration of authenticate(); adapters must not retain them.
// The HTTP/session integration supplies this interface in Stage D.3/D.4.
class AuthenticationRequest {
public:
    virtual ~AuthenticationRequest() = default;
    [[nodiscard]] virtual std::optional<std::string_view> header(
        std::string_view name) const = 0;
    [[nodiscard]] virtual std::optional<std::string_view> sessionValue(
        std::string_view key) const = 0;
};

enum class AuthenticationErrorCode {
    InvalidCredentials,
    ExpiredCredentials,
    ProviderUnavailable,
    InternalError,
    InvalidAdapterResult
};

// Do not include tokens, passwords or other credential material in errors.
struct AuthenticationError final {
    AuthenticationErrorCode code;
};

// A successful optional context means authenticated. std::nullopt means the
// adapter did not recognize credentials; Unexpected means recognized but failed.
using AuthenticationResult = Expected<std::optional<SecurityContext>, AuthenticationError>;

class AuthenticationAdapter {
public:
    virtual ~AuthenticationAdapter() = default;
    [[nodiscard]] virtual AuthenticationResult authenticate(
        const AuthenticationRequest& request) const = 0;
};

// Ordered chain: first authenticated context wins; no applicable adapters
// yield anonymous; a rejected credential stops evaluation (fail closed).
class AuthenticationPipeline final {
public:
    void add(std::shared_ptr<const AuthenticationAdapter> adapter) {
        if (adapter) {
            adapters_.push_back(std::move(adapter));
        }
    }

    [[nodiscard]] Expected<SecurityContext, AuthenticationError> authenticate(
        const AuthenticationRequest& request) const {
        for (const auto& adapter : adapters_) {
            AuthenticationResult result = Unexpected{
                AuthenticationError{AuthenticationErrorCode::InternalError}};
            try {
                result = adapter->authenticate(request);
            } catch (...) {
                return Unexpected{AuthenticationError{
                    AuthenticationErrorCode::InternalError}};
            }
            if (!result) {
                return Unexpected{result.error()};
            }
            if (!result.value()) {
                continue;
            }
            if (!result.value()->authenticated()) {
                return Unexpected{AuthenticationError{
                    AuthenticationErrorCode::InvalidAdapterResult}};
            }
            return *std::move(result.value());
        }
        return SecurityContext::anonymous();
    }

private:
    std::vector<std::shared_ptr<const AuthenticationAdapter>> adapters_;
};

} // namespace drogular::security