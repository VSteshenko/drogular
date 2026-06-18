#include <drogular/page.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>

class CoreTestingPage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Testing Page"));
    }

    std::string render(drogular::RenderContext& context) override {
        const auto title = context.require<std::string>("title");
        return "<h1>" + title + "</h1>";
    }
};

TEST(CoreTestingTests, RendersPageWithLifecycle) {
    const auto result = drogular::test::renderPage<CoreTestingPage>();

    EXPECT_EQ(result.html, "<h1>Testing Page</h1>");
    EXPECT_TRUE(result.context.contains("title"));
}

class CoreParentComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<section>Parent</section>";
    }

    std::vector<std::shared_ptr<drogular::Component>> children() override {
        return {
            std::make_shared<ChildComponent>()
        };
    }

private:
    class ChildComponent final : public drogular::Component {
    public:
        std::string render(drogular::RenderContext&) override {
            return "<p>Child</p>";
        }
    };
};

TEST(CoreTestingTests, RendersComponentTree) {
    CoreParentComponent component;
    drogular::RenderContext context;

    const auto html = drogular::test::renderComponentTree(component, context);

    EXPECT_TRUE(drogular::test::contains(html, "Parent"));
    EXPECT_TRUE(drogular::test::contains(html, "Child"));
}

class CoreSlotLayoutComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<main><slot/></main>";
    }

    std::vector<std::shared_ptr<drogular::Component>> children() override {
        return {
            std::make_shared<CoreSlotContentComponent>()
        };
    }

private:
    class CoreSlotContentComponent final : public drogular::Component {
    public:
        std::string render(drogular::RenderContext&) override {
            return "<h1>Hello Slot</h1>";
        }
    };
};

TEST(CoreTestingTests, RendersChildrenIntoSlot) {
    CoreSlotLayoutComponent component;
    drogular::RenderContext context;

    const auto html = drogular::test::renderComponentTree(component, context);

    EXPECT_EQ(html, "<main><h1>Hello Slot</h1></main>");
}

class CoreNamedSlotLayout final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<header><slot name=\"header\"/></header><main><slot name=\"content\"/></main>";
    }

    std::vector<std::shared_ptr<drogular::Component>> children() override {
        return {
            std::make_shared<CoreHeaderComponent>(),
            std::make_shared<CoreContentComponent>()
        };
    }

private:
    class CoreHeaderComponent final : public drogular::Component {
    public:
        std::string slot() const override {
            return "header";
        }

        std::string render(drogular::RenderContext&) override {
            return "<h1>Header</h1>";
        }
    };

    class CoreContentComponent final : public drogular::Component {
    public:
        std::string slot() const override {
            return "content";
        }

        std::string render(drogular::RenderContext&) override {
            return "<p>Content</p>";
        }
    };
};

TEST(CoreTestingTests, RendersNamedSlots) {
    CoreNamedSlotLayout layout;
    drogular::RenderContext context;

    const auto html = drogular::test::renderComponentTree(layout, context);

    EXPECT_EQ(html, "<header><h1>Header</h1></header><main><p>Content</p></main>");
}

class CoreScopedParentComponent final : public drogular::TemplateComponent {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Parent"));
    }

    std::string templateHtml() const override {
        return "<section>{{ title }}<slot/></section>";
    }

    std::vector<std::shared_ptr<drogular::Component>> children() override {
        auto child = std::make_shared<CoreScopedChildComponent>();
        child->setParam("title", std::string("Child"));
        return {child};
    }

private:
    class CoreScopedChildComponent final : public drogular::TemplateComponent {
    public:
        std::string templateHtml() const override {
            return "<p>{{ title }}</p>";
        }
    };
};

TEST(CoreTestingTests, ChildComponentParamsDoNotOverwriteParentContext) {
    CoreScopedParentComponent component;
    drogular::RenderContext context;

    const auto html =
        drogular::test::renderComponentTree(component, context);

    EXPECT_EQ(html, "<section>Parent<p>Child</p></section>");
}