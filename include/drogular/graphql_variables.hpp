#pragma once

#include <json/value.h>

#include <string>

namespace drogular {

class GraphQLVariables {
public:
    GraphQLVariables& set(
        const std::string& name,
        const std::string& value
    );

    GraphQLVariables& set(
        const std::string& name,
        int value
    );

    GraphQLVariables& set(
        const std::string& name,
        bool value
    );

    GraphQLVariables& set(
        const std::string& name,
        const Json::Value& value
    );

    GraphQLVariables& set(
        const std::string& name,
        const char* value
    );

    const Json::Value& json() const;

private:
    Json::Value values_;
};

} // namespace drogular