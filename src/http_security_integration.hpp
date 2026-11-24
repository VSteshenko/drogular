#pragma once

#include <drogular/detail/request_context_state.hpp>
#include <drogular/security/http_security_policy.hpp>
#include <drogular/session_store.hpp>

#include <drogon/drogon.h>

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace drogular::detail {

// Per-request facade. Cookie content is never treated as a Principal: the
// session must first be resolved through the server-side SessionStore.
class HttpAuthenticationRequest final : public security::AuthenticationRequest {
public:
    HttpAuthenticationRequest(const drogon::HttpRequestPtr& request,
                              ApplicationServices* services)
        : request_(request)
    {
        if (!request || !services) {
            return;
        }

        auto store = services->service<SessionStore>();
        if (!store) {
            return;
        }

        const auto sessionId = request->getCookie("session_id");
        if (!sessionId.empty()) {
            session_ = store->get(sessionId);
        }
    }

    std::optional<std::string_view> header(std::string_view name) const override {
        if (!request_) {
            return std::nullopt;
        }

        const auto [it, inserted] =
            headers_.try_emplace(std::string(name), std::nullopt);
        if (inserted) {
            const auto value = request_->getHeader(it->first);
            if (!value.empty()) {
                it->second = value;
            }
        }
        if (!it->second) {
            return std::nullopt;
        }
        return std::string_view{*it->second};
    }

    std::optional<std::string_view> sessionValue(std::string_view key) const override {
        if (!session_) {
            return std::nullopt;
        }

        const auto [it, inserted] =
            sessionValues_.try_emplace(std::string(key), std::nullopt);
        if (inserted) {
            it->second = session_->get(it->first);
        }
        if (!it->second) {
            return std::nullopt;
        }
        return std::string_view{*it->second};
    }

private:
    drogon::HttpRequestPtr request_;
    std::shared_ptr<Session> session_;
    mutable std::map<std::string, std::optional<std::string>> headers_;
    mutable std::map<std::string, std::optional<std::string>> sessionValues_;
};

[[nodiscard]] inline std::optional<int> authenticateAndAuthorize(
    const std::shared_ptr<RequestContextState>& state,
    const security::HttpSecurityConfiguration& configuration,
    const security::HttpSecurityPolicy& policy) {
    try {
        if (configuration.pipeline) {
            HttpAuthenticationRequest facade(state->request(), state->services());
            const auto result =
                configuration.pipeline->authenticate(facade);
            if (!result) {
                return security::authenticationErrorHttpStatus(result.error().code);
            }
            state->setSecurityContext(*result);
        }
        const auto decision = security::evaluateHttpPolicy(state->securityContext(), policy);
        if (decision == security::AuthorizationDecision::Allowed) {
            return std::nullopt;
        }
        return security::authorizationHttpStatus(decision);
    } catch (...) {
        // Never continue as anonymous after unexpected authentication failures.
        return 500;
    }
}

[[nodiscard]] inline drogon::HttpResponsePtr securityErrorResponse(int status) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    response->addHeader("Cache-Control", "no-store");
    response->setBody(status == 401 ? "Unauthorized" :
                      status == 403 ? "Forbidden" :
                      status == 503 ? "Service Unavailable" : "Internal Server Error");
    return response;
}

} // namespace drogular::detail