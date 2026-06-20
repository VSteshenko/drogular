#pragma once

#include <json/json.h>

#include <string>
#include <vector>

namespace drogular {

enum class ActionResultType {
    Empty,
    Redirect,
    Html,
    Json
};

struct Cookie {
    std::string name;
    std::string value;
    std::string path = "/";
    bool httpOnly = true;
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

    ActionResult& cookie(
        std::string name,
        std::string value,
        std::string path = "/",
        bool httpOnly = true
    );

    const std::vector<Cookie>& cookies() const;

private:
    ActionResultType type_ = ActionResultType::Empty;
    std::string location_;
    std::string body_;
    Json::Value json_;
    std::vector<Cookie> cookies_;
};

} // namespace drogular
