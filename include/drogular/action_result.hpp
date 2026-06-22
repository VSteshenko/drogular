#pragma once

#include <json/json.h>

#include <string>
#include <vector>
#include <filesystem>

namespace drogular {

enum class ActionResultType {
    Empty,
    Redirect,
    Html,
    Json,
    File
};

struct Cookie {
    std::string name;
    std::string value;
    std::string path = "/";
    bool httpOnly = true;
};

struct FileResponseInfo {
    std::filesystem::path path;
    std::string downloadName;
    bool forceDownload = false;
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

    static ActionResult file(
        std::filesystem::path path
    );

    static ActionResult download(
        std::filesystem::path path,
        std::string downloadName
    );

    const FileResponseInfo& fileInfo() const;

private:
    ActionResultType type_ = ActionResultType::Empty;
    std::string location_;
    std::string body_;
    Json::Value json_;
    std::vector<Cookie> cookies_;
    FileResponseInfo fileInfo_;
};

} // namespace drogular
