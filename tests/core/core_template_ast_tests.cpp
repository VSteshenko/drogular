#include <drogular/template_ast.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(CoreTemplateAstTests, CreatesTextNode) {
    TextNode node("Hello");

    EXPECT_EQ(node.type(), NodeType::Text);
    EXPECT_EQ(node.text(), "Hello");
}

TEST(CoreTemplateAstTests, CreatesVariableNode) {
    VariableNode node("title");

    EXPECT_EQ(node.type(), NodeType::Variable);
    EXPECT_EQ(node.expression(), "title");
}

TEST(CoreTemplateAstTests, CreatesRawVariableNode) {
    RawVariableNode node("html");

    EXPECT_EQ(node.type(), NodeType::RawVariable);
    EXPECT_EQ(node.expression(), "html");
}

TEST(CoreTemplateAstTests, CreatesIfNode) {
    IfNode node("show");

    node.trueBranch().push_back(
        std::make_shared<TextNode>("Yes")
    );

    node.falseBranch().push_back(
        std::make_shared<TextNode>("No")
    );

    EXPECT_EQ(node.type(), NodeType::If);
    EXPECT_EQ(node.condition(), "show");
    EXPECT_EQ(node.trueBranch().size(), 1);
    EXPECT_EQ(node.falseBranch().size(), 1);
}

TEST(CoreTemplateAstTests, CreatesForeachNode) {
    ForeachNode node("item in items");

    node.body().push_back(
        std::make_shared<VariableNode>("item")
    );

    EXPECT_EQ(node.type(), NodeType::Foreach);
    EXPECT_EQ(node.expression(), "item in items");
    EXPECT_EQ(node.body().size(), 1);
}

TEST(CoreTemplateAstTests, CreatesComponentNode) {
    ComponentNode node("<Card />");

    EXPECT_EQ(node.type(), NodeType::Component);
    EXPECT_EQ(node.tagHtml(), "<Card />");
}

TEST(CoreTemplateAstTests, CreatesForeachEmptyBranch) {
    ForeachNode node("item in items");
    node.emptyBranch().push_back(std::make_shared<TextNode>("Empty"));

    ASSERT_EQ(node.emptyBranch().size(), 1);
    EXPECT_EQ(node.emptyBranch()[0]->type(), NodeType::Text);
}

TEST(CoreTemplateAstTests, CreatesLoopControlNodes) {
    BreakNode breakNode;
    ContinueNode continueNode;

    EXPECT_EQ(breakNode.type(), NodeType::Break);
    EXPECT_EQ(continueNode.type(), NodeType::Continue);
}

TEST(CoreTemplateAstTests, CreatesLetNode) {
    LetNode node(
        "total",
        "projects.count()"
    );

    EXPECT_EQ(node.type(), NodeType::Let);
    EXPECT_EQ(node.name(), "total");
    EXPECT_EQ(node.expression(), "projects.count()");
}

TEST(CoreTemplateAstTests, CreatesConstNode) {
    ConstNode node(
        "PageSize",
        "20"
    );

    EXPECT_EQ(node.type(), NodeType::Const);
    EXPECT_EQ(node.name(), "PageSize");
    EXPECT_EQ(node.expression(), "20");
}

TEST(CoreTemplateAstTests, CreatesSwitchNode) {
    SwitchNode node("status");
    SwitchCase switchCase;
    switchCase.expressions = "\"Draft\", \"Pending\"";
    switchCase.body.push_back(std::make_shared<TextNode>("waiting"));
    node.cases().push_back(std::move(switchCase));
    node.defaultBranch().push_back(std::make_shared<TextNode>("other"));

    EXPECT_EQ(node.type(), NodeType::Switch);
    EXPECT_EQ(node.expression(), "status");
    ASSERT_EQ(node.cases().size(), 1);
    EXPECT_EQ(node.cases()[0].expressions, "\"Draft\", \"Pending\"");
    ASSERT_EQ(node.cases()[0].body.size(), 1);
    ASSERT_EQ(node.defaultBranch().size(), 1);
}