#include <drogular/page.hpp>
#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <string>
#include <json/json.h>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeTemplateFile(
    const std::string& name,
    const std::string& content
) {
    const auto path =
        std::filesystem::temp_directory_path() /
        name;

    std::ofstream file(path);
    file << content;

    return path;
}

} // namespace

class CoreTestTemplatePage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        context.set("title", std::string("Hello TemplatePage"));
    }

    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

TEST(CoreTemplatePageTests, RendersTemplateWithContext) {
    CoreTestTemplatePage page;
    drogular::RenderContext context;

    page.onInit(context);

    EXPECT_EQ(
        page.render(context),
        "<h1>Hello TemplatePage</h1>"
    );
}

class CoreTemplatePageCardComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreCard";

    std::string render(drogular::RenderContext&) override {
        return "<article>CoreCard from TemplatePage</article>";
    }
};

class CoreComponentTagTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return "<main><CoreCard /></main>";
    }
};

TEST(CoreTemplatePageTests, RendersComponentTags) {
    drogular::ApplicationServices services;
    services.components().registerComponent<CoreTemplatePageCardComponent>();

    CoreComponentTagTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article>CoreCard from TemplatePage</article></main>"
    );
}

class CoreTemplatePageCardWithInput final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreCardWithInput";

    std::string templateHtml() const override {
        return "<article>{{ title }}</article>";
    }
};

class CoreComponentInputTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return R"(<main><CoreCardWithInput title="Welcome from Page" /></main>)";
    }
};

TEST(CoreTemplatePageTests, RendersComponentTagsWithStringInputs) {
    drogular::ApplicationServices services;
    services.components().registerComponent<CoreTemplatePageCardWithInput>();

    CoreComponentInputTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article>Welcome from Page</article></main>"
    );
}

class CoreTemplateTodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreTodoItem";

    std::string templateHtml() const override {
        return "<li>{{ title }}</li>";
    }
};

class CoreComponentBindingForeachPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        Json::Value todos(Json::arrayValue);

        Json::Value first;
        first["title"] = "Learn Drogular";

        Json::Value second;
        second["title"] = "Build Components";

        todos.append(first);
        todos.append(second);

        context.set("todos", todos);
    }

    std::string templateHtml() const override {
        return R"(
<ul>
@foreach(todo in todos)
<CoreTodoItem title="{{ todo.title }}" />
@endforeach
</ul>
)";
    }
};

TEST(CoreTemplatePageTests, RendersComponentBindingsInsideForeach) {
    drogular::ApplicationServices services;
    services.components().registerComponent<CoreTemplateTodoItemComponent>();

    CoreComponentBindingForeachPage page;

    drogular::RenderContext context;
    context.setServices(&services);

    page.onInit(context);

    const auto html = page.render(context);

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<li>Learn Drogular</li>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<li>Build Components</li>"
        )
    );
}

class CoreSlotCardComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreSlotCard";

    std::string templateHtml() const override {
        return "<article><slot/></article>";
    }
};

class CoreComponentSlotTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return R"(<main><CoreSlotCard><p>Hello from page</p></CoreSlotCard></main>)";
    }
};

TEST(CoreTemplatePageTests, RendersComponentTagsWithDefaultSlot) {
    drogular::ApplicationServices services;
    services.components().registerComponent<CoreSlotCardComponent>();

    CoreComponentSlotTemplatePage page;
    drogular::RenderContext context;
    context.setServices(&services);

    EXPECT_EQ(
        page.render(context),
        "<main><article><p>Hello from page</p></article></main>"
    );
}

class CoreNestedPageCardComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreNestedPageCard";

    std::string templateHtml() const override {
        return "<article><slot/></article>";
    }
};

class CoreNestedPageButtonComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "CoreNestedPageButton";

    std::string templateHtml() const override {
        return "<button>{{ title }}</button>";
    }
};

class CoreNestedComponentTemplatePage final : public drogular::TemplatePage {
public:
    std::string templateHtml() const override {
        return R"(
<main>
<CoreNestedPageCard>
<CoreNestedPageButton title="Save" />
</CoreNestedPageCard>
</main>
)";
    }
};

TEST(CoreTemplatePageTests, RendersNestedComponentTags) {
    drogular::ApplicationServices services;

    services.components().registerComponent<CoreNestedPageCardComponent>();
    services.components().registerComponent<CoreNestedPageButtonComponent>();

    CoreNestedComponentTemplatePage page;

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html = page.render(context);

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<article>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            html,
            "<button>Save</button>"
        )
    );
}

class CoreTemplatePageInlinePage final
    : public drogular::TemplatePage
{
public:
    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }

    void onInit(
        drogular::RenderContext& context
    ) override {
        context.set(
            "title",
            std::string("Inline Template")
        );
    }
};

TEST(CoreTemplatePageTests, UsesTemplateHtmlWhenTemplatePathIsEmpty) {
    const auto result =
        drogular::test::renderPage<CoreTemplatePageInlinePage>();

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<h1>Inline Template</h1>"
        )
    );
}

class CoreTemplatePageExternalPage final
    : public drogular::TemplatePage
{
public:
    static inline std::string path;

    std::string templatePath() const override {
        return path;
    }

    void onInit(
        drogular::RenderContext& context
    ) override {
        context.set(
            "title",
            std::string("External Template")
        );
    }
};

TEST(CoreTemplatePageTests, UsesTemplatePathWhenProvided) {
    const auto path =
        writeTemplateFile(
            "drogular_template_page_external.html",
            "<h1>{{ title }}</h1>"
        );

    CoreTemplatePageExternalPage::path = path.string();

    const auto result =
        drogular::test::renderPage<CoreTemplatePageExternalPage>();

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<h1>External Template</h1>"
        )
    );

    std::filesystem::remove(path);
}

class CoreTemplatePagePriorityPage final
    : public drogular::TemplatePage
{
public:
    static inline std::string path;

    std::string templatePath() const override {
        return path;
    }

    std::string templateHtml() const override {
        return "<h1>Inline Template</h1>";
    }
};

TEST(CoreTemplatePageTests, TemplatePathOverridesTemplateHtml) {
    const auto path =
        writeTemplateFile(
            "drogular_template_page_priority.html",
            "<h1>External Template</h1>"
        );

    CoreTemplatePagePriorityPage::path = path.string();

    const auto result =
        drogular::test::renderPage<CoreTemplatePagePriorityPage>();

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<h1>External Template</h1>"
        )
    );

    EXPECT_FALSE(
        drogular::test::contains(
            result.html,
            "Inline Template"
        )
    );

    std::filesystem::remove(path);
}

TEST(CoreTemplatePageTests, UsesTemplateRootFromApplicationOptions) {
    const auto root =
        std::filesystem::temp_directory_path();

    const auto path =
        writeTemplateFile(
            "drogular_template_root_page.html",
            "<h1>{{ title }}</h1>"
        );

    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    options.setTemplateRoot(root);
    services.setOptions(&options);

    CoreTemplatePageExternalPage::path =
        "drogular_template_root_page.html";

    const auto result =
        drogular::test::renderPage<
            CoreTemplatePageExternalPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<h1>External Template</h1>"
        )
    );

    std::filesystem::remove(path);
}

class CoreTemplatePageCachePage final
    : public drogular::TemplatePage
{
public:
    static inline std::string path;

    std::string templatePath() const override {
        return path;
    }
};

TEST(CoreTemplatePageTests, CachesExternalTemplateSource) {
    const auto path =
        writeTemplateFile(
            "drogular_template_page_cache.html",
            "<h1>First</h1>"
        );

    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    options.setTemplateRoot(
        std::filesystem::temp_directory_path()
    );

    services.setOptions(&options);

    CoreTemplatePageCachePage::path =
        "drogular_template_page_cache.html";

    const auto first =
        drogular::test::renderPage<
            CoreTemplatePageCachePage
        >(&services);

    {
        std::ofstream file(path);
        file << "<h1>Second</h1>";
    }

    const auto second =
        drogular::test::renderPage<
            CoreTemplatePageCachePage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            first.html,
            "<h1>First</h1>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            second.html,
            "<h1>First</h1>"
        )
    );

    EXPECT_FALSE(
        drogular::test::contains(
            second.html,
            "<h1>Second</h1>"
        )
    );

    std::filesystem::remove(path);
}

TEST(CoreTemplatePageTests, ReloadsExternalTemplateWhenCacheIsDisabled) {
    const auto path =
        writeTemplateFile(
            "drogular_template_page_reload.html",
            "<h1>First</h1>"
        );

    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    options.setTemplateRoot(
        std::filesystem::temp_directory_path()
    );

    options.setTemplateCacheEnabled(false);

    services.setOptions(&options);

    CoreTemplatePageCachePage::path =
        "drogular_template_page_reload.html";

    const auto first =
        drogular::test::renderPage<
            CoreTemplatePageCachePage
        >(&services);

    {
        std::ofstream file(path);
        file << "<h1>Second</h1>";
    }

    const auto second =
        drogular::test::renderPage<
            CoreTemplatePageCachePage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            first.html,
            "<h1>First</h1>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            second.html,
            "<h1>Second</h1>"
        )
    );

    std::filesystem::remove(path);
}

class CoreTemplatePageWithIncludePage final
    : public drogular::TemplatePage
{
public:
    static inline std::string path;

    std::string templatePath() const override {
        return path;
    }
};

TEST(CoreTemplatePageTests, ProcessesIncludesFromExternalTemplate) {
    const auto root =
        std::filesystem::temp_directory_path();

    const auto partial =
        writeTemplateFile(
            "drogular_template_page_partial.html",
            "<header>Header</header>"
        );

    const auto page =
        writeTemplateFile(
            "drogular_template_page_with_include.html",
            R"(@include("drogular_template_page_partial.html")
<main>Content</main>)"
        );

    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;

    options.setTemplateRoot(root);
    services.setOptions(&options);

    CoreTemplatePageWithIncludePage::path =
        "drogular_template_page_with_include.html";

    const auto result =
        drogular::test::renderPage<
            CoreTemplatePageWithIncludePage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<header>Header</header>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<main>Content</main>"
        )
    );

    std::filesystem::remove(partial);
    std::filesystem::remove(page);
}