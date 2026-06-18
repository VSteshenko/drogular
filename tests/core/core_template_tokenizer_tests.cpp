#include <drogular/template_tokenizer.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(CoreCoreTemplateTokenizerTests, TokenizesText) {
    const auto tokens = tokenize("Hello");

    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type, TokenType::Text);
    EXPECT_EQ(tokens[0].value, "Hello");
}

TEST(CoreTemplateTokenizerTests, TokenizesVariable) {
    const auto tokens = tokenize("<h1>{{ title }}</h1>");

    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[1].type, TokenType::Variable);
    EXPECT_EQ(tokens[1].value, " title ");
}

TEST(CoreTemplateTokenizerTests, TokenizesRawVariable) {
    const auto tokens = tokenize("{{{ html }}}");

    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type, TokenType::RawVariable);
    EXPECT_EQ(tokens[0].value, " html ");
}

TEST(CoreTemplateTokenizerTests, TokenizesIfElseEndIf) {
    const auto tokens =
        tokenize("@if(show)<p>Yes</p>@else<p>No</p>@endif");

    ASSERT_EQ(tokens.size(), 5);
    EXPECT_EQ(tokens[0].type, TokenType::If);
    EXPECT_EQ(tokens[0].value, "show");
    EXPECT_EQ(tokens[2].type, TokenType::Else);
    EXPECT_EQ(tokens[4].type, TokenType::EndIf);
}

TEST(CoreTemplateTokenizerTests, TokenizesForeach) {
    const auto tokens =
        tokenize("@foreach(item in items)<p>{{ item }}</p>@endforeach");

    ASSERT_EQ(tokens.size(), 5);
    EXPECT_EQ(tokens[0].type, TokenType::Foreach);
    EXPECT_EQ(tokens[0].value, "item in items");
    EXPECT_EQ(tokens[4].type, TokenType::EndForeach);
}

TEST(CoreTemplateTokenizerTests, TokenizesComponentTag) {
    const auto tokens = tokenize("<Card title=\"Hello\" />");

    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type, TokenType::ComponentTag);
    EXPECT_EQ(tokens[0].value, "<Card title=\"Hello\" />");
}