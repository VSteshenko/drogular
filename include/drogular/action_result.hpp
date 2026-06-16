#pragma once

#include <json/json.h>

#include <string>

namespace drogular {

enum class ActionResultType {
    Empty,
    Redirect,
    Html,
    Json
};

/**
 * Represents the result of an action handler.
 */
class ActionResult {
public:
    static ActionResult empty();

    static ActionResult redirect(std::string location);

    static ActionResult html(std::string html);

    static ActionResult json(Json::Value json);

    ActionResultType type() const;

    const std::string& location() const;

    const std::string& body() const;

    const Json::Value& json() const;

private:
    ActionResultType type_ = ActionResultType::Empty;
    std::string location_;
    std::string body_;
    Json::Value json_;
};

} // namespace drogular
