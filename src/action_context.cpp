#include <drogular/action_context.hpp>

namespace drogular {

ActionContext::ActionContext(
    drogon::HttpRequestPtr request,
    ApplicationServices* services
)
    : request_(std::move(request)),
      services_(services) {
}

const drogon::HttpRequestPtr& ActionContext::request() const {
    return request_;
}

ApplicationServices* ActionContext::services() {
    return services_;
}

const ApplicationServices* ActionContext::services() const {
    return services_;
}

std::optional<std::string> ActionContext::formValue(
    const std::string& name
) const {
    const auto value = request_->getParameter(name);

    if (value.empty()) {
        return std::nullopt;
    }

    return value;
}

} // namespace drogular
