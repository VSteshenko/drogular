#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class CoreTestPage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext&) override {
        title_ = "Hello";
    }

    std::string render(drogular::RenderContext&) override {
        return "<h1>" + title_ + "</h1>";
    }

private:
    std::string title_;
};

TEST(CoreComponentTests, PageCallsLifecycleBeforeRender) {
    drogular::RenderContext context;
    CoreTestPage page;

    page.onInit(context);

    EXPECT_EQ(page.render(context), "<h1>Hello</h1>");
}

TEST(CoreComponentTests, PageHasNoQueryByDefault) {
    CoreTestPage page;

    EXPECT_FALSE(page.query().has_value());
}

TEST(CoreComponentTests, StoresAndReadsComponentParameter) {
    class ParamComponent final : public drogular::Component {
    public:
        std::string render(drogular::RenderContext&) override {
            return "";
        }
    };

    ParamComponent component;

    component.setParam("title", std::string("Hello"));

    const auto title = component.param<std::string>("title");

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, "Hello");
}

TEST(CoreComponentTests, ReturnsNulloptForMissingComponentParameter) {
    class ParamComponent final : public drogular::Component {
    public:
        std::string render(drogular::RenderContext&) override {
            return "";
        }
    };

    ParamComponent component;

    EXPECT_FALSE(component.param<std::string>("missing").has_value());
}

TEST(CoreComponentTests, StoresAndReadsComponentInput) {
    class InputComponent final : public drogular::Component {
    public:
        std::string render(drogular::RenderContext&) override {
            return "";
        }
    };

    InputComponent component;

    component.setInput("title", std::string("Hello Input"));

    const auto title = component.input<std::string>("title");

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, "Hello Input");
}