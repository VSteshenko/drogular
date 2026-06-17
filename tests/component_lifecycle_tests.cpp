#include <drogular/component.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

class LifecycleTestComponent final : public drogular::Component {
public:
    std::vector<std::string>* events = nullptr;

    void onInit(drogular::RenderContext&) override {
        events->push_back("init");
    }

    std::string render(drogular::RenderContext&) override {
        events->push_back("render");
        return "<p>Hello</p>";
    }

    void onDestroy(drogular::RenderContext&) override {
        events->push_back("destroy");
    }
};

TEST(ComponentLifecycleTests, CallsLifecycleHooksAroundRender) {
    std::vector<std::string> events;

    LifecycleTestComponent component;
    component.events = &events;

    drogular::RenderContext context;

    const auto html =
        drogular::test::renderComponentTree(
            component,
            context
        );

    EXPECT_EQ(html, "<p>Hello</p>");

    ASSERT_EQ(events.size(), 3);
    EXPECT_EQ(events[0], "init");
    EXPECT_EQ(events[1], "render");
    EXPECT_EQ(events[2], "destroy");
}
