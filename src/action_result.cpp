#include <drogular/action_result.hpp>

#include <utility>

namespace drogular {

ActionResult ActionResult::empty() {
    return {};
}

ActionResult ActionResult::redirect(std::string location) {
    ActionResult result;
    result.type_ = ActionResultType::Redirect;
    result.location_ = std::move(location);
    return result;
}

ActionResult ActionResult::html(std::string html) {
    ActionResult result;
    result.type_ = ActionResultType::Html;
    result.body_ = std::move(html);
    return result;
}

ActionResult ActionResult::json(Json::Value json) {
    ActionResult result;
    result.type_ = ActionResultType::Json;
    result.json_ = std::move(json);
    return result;
}

ActionResultType ActionResult::type() const {
    return type_;
}

const std::string& ActionResult::location() const {
    return location_;
}

const std::string& ActionResult::body() const {
    return body_;
}

const Json::Value& ActionResult::json() const {
    return json_;
}

} // namespace drogular
