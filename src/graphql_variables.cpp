#include <drogular/graphql_variables.hpp>

#include <string>

namespace drogular {

GraphQLVariables& GraphQLVariables::set(
    const std::string& name,
    const std::string& value
) {
    values_[name] = value;
    return *this;
}

GraphQLVariables& GraphQLVariables::set(
    const std::string& name,
    int value
) {
    values_[name] = value;
    return *this;
}

GraphQLVariables& GraphQLVariables::set(
    const std::string& name,
    bool value
) {
    values_[name] = value;
    return *this;
}

GraphQLVariables& GraphQLVariables::set(
    const std::string& name,
    const Json::Value& value
) {
    values_[name] = value;
    return *this;
}

GraphQLVariables& GraphQLVariables::set(
    const std::string& name,
    const char* value
) {
    values_[name] = value != nullptr
        ? std::string(value)
        : std::string("");

    return *this;
}

const Json::Value& GraphQLVariables::json() const {
    return values_;
}

} // namespace drogular