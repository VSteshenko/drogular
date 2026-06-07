#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class TestPage final : public drogular::Page {
public:
    std::string render() override {
        return "<h1>Hello</h1>";
    }
};

TEST(ComponentTests, PageRendersHtml) {
    TestPage page;

    EXPECT_EQ(
        page.render(),
        "<h1>Hello</h1>"
    );
}
