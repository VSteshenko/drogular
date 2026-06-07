#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class TestPage final : public drogular::Page {
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

TEST(ComponentTests, PageCallsLifecycleBeforeRender) {
    drogular::RenderContext context;
    TestPage page;

    page.onInit(context);

    EXPECT_EQ(
        page.render(context),
        "<h1>Hello</h1>"
    );
}

TEST(ComponentTests, PageHasNoQueryByDefault) {
    TestPage page;

    EXPECT_FALSE(page.query().has_value());
}
