#include <drogular/template_ast.hpp>

#include <utility>

namespace drogular::template_compiler {

TextNode::TextNode(std::string text)
    : text_(std::move(text)) {
}

NodeType TextNode::type() const {
    return NodeType::Text;
}

const std::string& TextNode::text() const {
    return text_;
}

VariableNode::VariableNode(std::string expression)
    : expression_(std::move(expression)) {
}

NodeType VariableNode::type() const {
    return NodeType::Variable;
}

const std::string& VariableNode::expression() const {
    return expression_;
}

RawVariableNode::RawVariableNode(std::string expression)
    : expression_(std::move(expression)) {
}

NodeType RawVariableNode::type() const {
    return NodeType::RawVariable;
}

const std::string& RawVariableNode::expression() const {
    return expression_;
}

IfNode::IfNode(std::string condition)
    : condition_(std::move(condition)) {
}

NodeType IfNode::type() const {
    return NodeType::If;
}

const std::string& IfNode::condition() const {
    return condition_;
}

std::vector<NodePtr>& IfNode::trueBranch() {
    return trueBranch_;
}

std::vector<NodePtr>& IfNode::falseBranch() {
    return falseBranch_;
}

const std::vector<NodePtr>& IfNode::trueBranch() const {
    return trueBranch_;
}

const std::vector<NodePtr>& IfNode::falseBranch() const {
    return falseBranch_;
}

ForeachNode::ForeachNode(std::string expression)
    : expression_(std::move(expression)) {
}

NodeType ForeachNode::type() const {
    return NodeType::Foreach;
}

const std::string& ForeachNode::expression() const {
    return expression_;
}

std::vector<NodePtr>& ForeachNode::body() {
    return body_;
}

const std::vector<NodePtr>& ForeachNode::body() const {
    return body_;
}

ComponentNode::ComponentNode(std::string tagHtml)
    : tagHtml_(std::move(tagHtml)) {
}

NodeType ComponentNode::type() const {
    return NodeType::Component;
}

const std::string& ComponentNode::tagHtml() const {
    return tagHtml_;
}

} // namespace drogular::template_compiler
