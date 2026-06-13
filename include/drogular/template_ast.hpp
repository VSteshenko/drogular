#pragma once

#include <memory>
#include <string>
#include <vector>

namespace drogular::template_compiler {

enum class NodeType {
    Text,
    Variable,
    RawVariable,
    If,
    Foreach,
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

    NodeType type() const override;

    const std::string& condition() const;

    std::vector<NodePtr>& trueBranch();
    std::vector<NodePtr>& falseBranch();

    const std::vector<NodePtr>& trueBranch() const;
    const std::vector<NodePtr>& falseBranch() const;

private:
    std::string condition_;
    std::vector<NodePtr> trueBranch_;
    std::vector<NodePtr> falseBranch_;
};

class ForeachNode final : public Node {
public:
    explicit ForeachNode(std::string expression);

    NodeType type() const override;

    const std::string& expression() const;

    std::vector<NodePtr>& body();
    const std::vector<NodePtr>& body() const;

private:
    std::string expression_;
    std::vector<NodePtr> body_;
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
