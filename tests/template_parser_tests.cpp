#include <drogular/template_parser.hpp>
#include <drogular/template_tokenizer.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(TemplateParserTests, ParsesTextNode) {
    const auto nodes = parse(tokenize("Hello"));

    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0]->type(), NodeType::Text);
}

TEST(TemplateParserTests, ParsesVariableNode) {
    const auto nodes = parse(tokenize("{{ title }}"));

    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0]->type(), NodeType::Variable);
}

TEST(TemplateParserTests, ParsesRawVariableNode) {
    const auto nodes = parse(tokenize("{{{ html }}}"));

    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0]->type(), NodeType::RawVariable);
}

TEST(TemplateParserTests, ParsesComponentNode) {
    const auto nodes = parse(tokenize("<Card />"));

    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0]->type(), NodeType::Component);
}

TEST(TemplateParserTests, ParsesIfNode) {
    const auto nodes = parse(tokenize("@if(show)Hello@endif"));

    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->type(), NodeType::If);

    const auto ifNode =
        std::dynamic_pointer_cast<IfNode>(nodes[0]);

    ASSERT_NE(ifNode, nullptr);
    EXPECT_EQ(ifNode->condition(), "show");
    EXPECT_EQ(ifNode->trueBranch().size(), 1);
}

TEST(TemplateParserTests, ParsesForeachNode) {
    const auto nodes =
        parse(tokenize("@foreach(item in items){{ item }}@endforeach"));

    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->type(), NodeType::Foreach);

    const auto foreachNode =
        std::dynamic_pointer_cast<ForeachNode>(nodes[0]);

    ASSERT_NE(foreachNode, nullptr);
    EXPECT_EQ(foreachNode->expression(), "item in items");
    EXPECT_EQ(foreachNode->body().size(), 1);
}
