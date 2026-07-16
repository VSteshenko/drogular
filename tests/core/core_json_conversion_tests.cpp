#include <drogular/json_conversion.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

TEST(CoreJsonConversionTests, ConvertsPrimitiveValues) {
    EXPECT_EQ(
        drogular::toJsonValue(
            std::string("hello")
        ).asString(),
        "hello"
    );

    EXPECT_EQ(
        drogular::toJsonValue(42).asInt(),
        42
    );

    EXPECT_TRUE(
        drogular::toJsonValue(true).asBool()
    );
}

TEST(CoreJsonConversionTests, ConvertsOptionalValue) {
    const std::optional<int> value =
        42;

    EXPECT_EQ(
        drogular::toJsonValue(value).asInt(),
        42
    );
}

TEST(CoreJsonConversionTests, ConvertsEmptyOptionalToNull) {
    const std::optional<int> value;

    EXPECT_TRUE(
        drogular::toJsonValue(value).isNull()
    );
}

TEST(CoreJsonConversionTests, ConvertsVectorToJsonArray) {
    const std::vector<std::string> values = {
        "one",
        "two"
    };

    const auto json =
        drogular::toJsonValue(values);

    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(json.size(), 2);

    EXPECT_EQ(
        json[0].asString(),
        "one"
    );

    EXPECT_EQ(
        json[1].asString(),
        "two"
    );
}

namespace test_models {

struct Person {
    std::string name;
    int age = 0;
};

Json::Value toJson(
    const Person& person
) {
    Json::Value json(Json::objectValue);

    json["name"] = person.name;
    json["age"] = person.age;

    return json;
}

} // namespace test_models

TEST(CoreJsonConversionTests, ConvertsCustomStructureUsingAdl) {
    const test_models::Person person{
        .name = "Anna",
        .age = 12
    };

    const auto json =
        drogular::toJsonValue(person);

    EXPECT_EQ(
        json["name"].asString(),
        "Anna"
    );

    EXPECT_EQ(
        json["age"].asInt(),
        12
    );
}