#include <drogular/template_ast.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(TemplateAstTests, CreatesTextNode) {
    TextNode node("Hello");

    EXPECT_EQ(node.type(), NodeType::Text);
    EXPECT_EQ(node.text(), "Hello");
}

TEST(TemplateAstTests, CreatesVariableNode) {
    VariableNode node("title");

    EXPECT_EQ(node.type(), NodeType::Variable);
    EXPECT_EQ(node.expression(), "title");
}

TEST(TemplateAstTests, CreatesRawVariableNode) {
    RawVariableNode node("html");

    EXPECT_EQ(node.type(), NodeType::RawVariable);
    EXPECT_EQ(node.expression(), "html");
}

TEST(TemplateAstTests, CreatesIfNode) {
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

TEST(TemplateAstTests, CreatesForeachNode) {
    ForeachNode node("item in items");

    node.body().push_back(
        std::make_shared<VariableNode>("item")
    );

    EXPECT_EQ(node.type(), NodeType::Foreach);
    EXPECT_EQ(node.expression(), "item in items");
    EXPECT_EQ(node.body().size(), 1);
}

TEST(TemplateAstTests, CreatesComponentNode) {
    ComponentNode node("<Card />");

    EXPECT_EQ(node.type(), NodeType::Component);
    EXPECT_EQ(node.tagHtml(), "<Card />");
}
