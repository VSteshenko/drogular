#include <drogular/request_parameters.hpp>

#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <initializer_list>
#include <utility>

namespace {

drogon::HttpRequestPtr makeRequest(
    std::initializer_list<std::pair<const char*, const char*>> values
) {
    auto request =
        drogon::HttpRequest::newHttpRequest();

    for (const auto& [name, value] : values) {
        request->setParameter(name, value);
    }

    return request;
}

} // namespace

TEST(RequestParametersTests, ReadsStringAndIntegerValues) {
    const drogular::RequestParameters parameters(makeRequest({
        {"search", "hello"},
        {"page", "-2"},
        {"pageSize", "25"}
    }));

    EXPECT_EQ(parameters.value("search"), "hello");
    EXPECT_EQ(parameters.optionalString("search"), "hello");
    EXPECT_EQ(parameters.integer("page"), -2);
    EXPECT_FALSE(parameters.positiveInteger("page").has_value());
    EXPECT_EQ(
        parameters.positiveIntegerOr("pageSize", 10),
        25
    );
}

TEST(RequestParametersTests, UsesFallbackForMissingOrInvalidValues) {
    const drogular::RequestParameters parameters(makeRequest({
        {"page", "invalid"},
        {"pageSize", "0"}
    }));

    EXPECT_FALSE(parameters.optionalString("search").has_value());
    EXPECT_EQ(
        parameters.integerOr("page", 7),
        7
    );
    EXPECT_EQ(
        parameters.positiveIntegerOr("pageSize", 10),
        10
    );
}

TEST(RequestParametersTests, BoundsPositiveInteger) {
    const drogular::RequestParameters parameters(makeRequest({
        {"pageSize", "250"}
    }));

    EXPECT_EQ(
        parameters.boundedPositiveIntegerOr(
            "pageSize",
            10,
            100
        ),
        100
    );
}

TEST(RequestParametersTests, HandlesNullRequest) {
    const drogular::RequestParameters parameters(nullptr);

    EXPECT_TRUE(parameters.value("search").empty());
    EXPECT_EQ(
        parameters.positiveIntegerOr("page", 1),
        1
    );
}