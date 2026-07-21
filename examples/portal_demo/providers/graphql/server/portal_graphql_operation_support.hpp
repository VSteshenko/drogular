#pragma once

#include <drogular/graphql_response.hpp>
#include <json/value.h>

#include <algorithm>
#include <cctype>
#include <string>

class PortalGraphQLOperationSupport {
public:
    static std::string lowercase(std::string value) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }

    static drogular::GraphQLResponse response(Json::Value data) {
        Json::Value root(Json::objectValue);
        root["data"] = std::move(data);
        return drogular::GraphQLResponse(root);
    }
};