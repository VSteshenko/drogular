#pragma once

#include <drogular/template/binding.hpp>
#include <drogular/template/expression/ast.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace drogular::template_compiler {

enum class NodeType {
    Text,
    Variable,
    RawVariable,
    If,
    Foreach,
    Let,
    Const,
    Switch,
    Break,
    Continue,
    Component
};

class Node {
public:
    virtual ~Node() = default;

    virtual NodeType type() const = 0;
};

using NodePtr = std::shared_ptr<Node>;

class TextNode final : public Node {
public:
    explicit TextNode(std::string text);

    NodeType type() const override;

    const std::string& text() const;

private:
    std::string text_;
};

class VariableNode final : public Node {
public:
    explicit VariableNode(std::string expression);

    NodeType type() const override;

    const std::string& expression() const;

private:
    std::string expression_;
};

class RawVariableNode final : public Node {
public:
    explicit RawVariableNode(std::string expression);

    NodeType type() const override;

    const std::string& expression() const;

private:
    std::string expression_;
};

class IfNode final : public Node {
public:
    explicit IfNode(std::string condition);

    IfNode(
        std::string condition,
        template_expression::ExpressionPtr conditionExpression
    );

    NodeType type() const override;

    const std::string& condition() const;
    const template_expression::ExpressionPtr& conditionExpression() const;

    std::vector<NodePtr>& trueBranch();
    std::vector<NodePtr>& falseBranch();

    const std::vector<NodePtr>& trueBranch() const;
    const std::vector<NodePtr>& falseBranch() const;

private:
    std::string condition_;
    template_expression::ExpressionPtr conditionExpression_;
    std::vector<NodePtr> trueBranch_;
    std::vector<NodePtr> falseBranch_;
};

class BindingNode : public Node {
public:
    BindingNode(
        NodeType nodeType,
        std::string name,
        std::string expression,
        template_engine::BindingMutability mutability
    );

    BindingNode(
        NodeType nodeType,
        std::string name,
        std::string expression,
        template_expression::ExpressionPtr compiledExpression,
        template_engine::BindingMutability mutability
    );

    NodeType type() const override;

    const std::string& name() const;
    const std::string& expression() const;
    const template_expression::ExpressionPtr& compiledExpression() const;
    template_engine::BindingMutability mutability() const;

private:
    NodeType nodeType_;
    std::string name_;
    std::string expression_;
    template_expression::ExpressionPtr compiledExpression_;
    template_engine::BindingMutability mutability_;
};

class LetNode final : public BindingNode {
public:
    LetNode(std::string name, std::string expression);
    LetNode(
        std::string name,
        std::string expression,
        template_expression::ExpressionPtr compiledExpression
    );
};

class ConstNode final : public BindingNode {
public:
    ConstNode(std::string name, std::string expression);
    ConstNode(
        std::string name,
        std::string expression,
        template_expression::ExpressionPtr compiledExpression
    );
};

struct SwitchCase {
    std::string expressions;
    template_expression::ExpressionPtr compiledExpressions;
    std::vector<NodePtr> body;
};

class SwitchNode final : public Node {
public:
    explicit SwitchNode(std::string expression);

    SwitchNode(
        std::string expression,
        template_expression::ExpressionPtr compiledExpression
    );

    NodeType type() const override;

    const std::string& expression() const;
    const template_expression::ExpressionPtr& compiledExpression() const;

    std::vector<SwitchCase>& cases();
    const std::vector<SwitchCase>& cases() const;

    std::vector<NodePtr>& defaultBranch();
    const std::vector<NodePtr>& defaultBranch() const;

private:
    std::string expression_;
    template_expression::ExpressionPtr compiledExpression_;
    std::vector<SwitchCase> cases_;
    std::vector<NodePtr> defaultBranch_;
};

class ForeachNode final : public Node {
public:
    explicit ForeachNode(std::string expression);

    ForeachNode(
        std::string expression,
        std::string variable,
        std::string collection,
        template_expression::ExpressionPtr collectionExpression,
        std::optional<std::string> condition,
        template_expression::ExpressionPtr conditionExpression
    );

    NodeType type() const override;

    const std::string& expression() const;
    const std::string& variable() const;
    const std::string& collection() const;
    const template_expression::ExpressionPtr& collectionExpression() const;
    const std::optional<std::string>& condition() const;
    const template_expression::ExpressionPtr& conditionExpression() const;

    std::vector<NodePtr>& body();
    const std::vector<NodePtr>& body() const;

    std::vector<NodePtr>& emptyBranch();
    const std::vector<NodePtr>& emptyBranch() const;

private:
    std::string expression_;
    std::string variable_;
    std::string collection_;
    template_expression::ExpressionPtr collectionExpression_;
    std::optional<std::string> condition_;
    template_expression::ExpressionPtr conditionExpression_;
    std::vector<NodePtr> body_;
    std::vector<NodePtr> emptyBranch_;
};

class BreakNode final : public Node {
public:
    NodeType type() const override;
};

class ContinueNode final : public Node {
public:
    NodeType type() const override;
};

class ComponentNode final : public Node {
public:
    explicit ComponentNode(std::string tagHtml);

    NodeType type() const override;

    const std::string& tagHtml() const;

private:
    std::string tagHtml_;
};

} // namespace drogular::template_compiler