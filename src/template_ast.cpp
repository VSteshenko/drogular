#include <drogular/template_ast.hpp>
#include <drogular/template_runtime.hpp>

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
    if (const auto parsed = template_expression::parse(condition_)) {
        conditionExpression_ = parsed.expression;
    }
}

IfNode::IfNode(
    std::string condition,
    template_expression::ExpressionPtr conditionExpression
)
    : condition_(std::move(condition)),
      conditionExpression_(std::move(conditionExpression)) {
}

NodeType IfNode::type() const {
    return NodeType::If;
}

const std::string& IfNode::condition() const {
    return condition_;
}

const template_expression::ExpressionPtr& IfNode::conditionExpression() const {
    return conditionExpression_;
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


LetNode::LetNode(std::string name, std::string expression)
    : name_(std::move(name)),
      expression_(std::move(expression))
{
    if (const auto parsed = template_expression::parse(expression_)) {
        compiledExpression_ = parsed.expression;
    }
}

LetNode::LetNode(
    std::string name,
    std::string expression,
    template_expression::ExpressionPtr compiledExpression
)
    : name_(std::move(name)),
      expression_(std::move(expression)),
      compiledExpression_(std::move(compiledExpression))
{
}

NodeType LetNode::type() const {
    return NodeType::Let;
}

const std::string& LetNode::name() const {
    return name_;
}

const std::string& LetNode::expression() const {
    return expression_;
}

const template_expression::ExpressionPtr& LetNode::compiledExpression() const {
    return compiledExpression_;
}

ConstNode::ConstNode(std::string name, std::string expression)
    : name_(std::move(name)),
      expression_(std::move(expression))
{
    if (const auto parsed = template_expression::parse(expression_)) {
        compiledExpression_ = parsed.expression;
    }
}

ConstNode::ConstNode(
    std::string name,
    std::string expression,
    template_expression::ExpressionPtr compiledExpression
)
    : name_(std::move(name)),
      expression_(std::move(expression)),
      compiledExpression_(std::move(compiledExpression))
{
}

NodeType ConstNode::type() const {
    return NodeType::Const;
}

const std::string& ConstNode::name() const {
    return name_;
}

const std::string& ConstNode::expression() const {
    return expression_;
}

const template_expression::ExpressionPtr& ConstNode::compiledExpression() const {
    return compiledExpression_;
}

SwitchNode::SwitchNode(std::string expression)
    : expression_(std::move(expression)) {
    if (const auto parsed = template_expression::parse(expression_)) {
        compiledExpression_ = parsed.expression;
    }
}

SwitchNode::SwitchNode(
    std::string expression,
    template_expression::ExpressionPtr compiledExpression
)
    : expression_(std::move(expression)),
      compiledExpression_(std::move(compiledExpression)) {
}

NodeType SwitchNode::type() const {
    return NodeType::Switch;
}

const std::string& SwitchNode::expression() const {
    return expression_;
}

const template_expression::ExpressionPtr& SwitchNode::compiledExpression() const {
    return compiledExpression_;
}

std::vector<SwitchCase>& SwitchNode::cases() {
    return cases_;
}

const std::vector<SwitchCase>& SwitchNode::cases() const {
    return cases_;
}

std::vector<NodePtr>& SwitchNode::defaultBranch() {
    return defaultBranch_;
}

const std::vector<NodePtr>& SwitchNode::defaultBranch() const {
    return defaultBranch_;
}

ForeachNode::ForeachNode(std::string expression)
    : expression_(std::move(expression))
{
    if (const auto parsed = parseForeachExpression(expression_)) {
        variable_ = parsed->variable;
        collection_ = parsed->collection;
        collectionExpression_ = parsed->collectionExpression;
        condition_ = parsed->condition;
        conditionExpression_ = parsed->conditionExpression;
    }
}

ForeachNode::ForeachNode(
    std::string expression,
    std::string variable,
    std::string collection,
    template_expression::ExpressionPtr collectionExpression,
    std::optional<std::string> condition,
    template_expression::ExpressionPtr conditionExpression
)
    : expression_(std::move(expression)),
      variable_(std::move(variable)),
      collection_(std::move(collection)),
      collectionExpression_(std::move(collectionExpression)),
      condition_(std::move(condition)),
      conditionExpression_(std::move(conditionExpression))
{
}

NodeType ForeachNode::type() const {
    return NodeType::Foreach;
}

const std::string& ForeachNode::expression() const {
    return expression_;
}

const std::string& ForeachNode::variable() const {
    return variable_;
}

const std::string& ForeachNode::collection() const {
    return collection_;
}

const template_expression::ExpressionPtr& ForeachNode::collectionExpression() const {
    return collectionExpression_;
}

const std::optional<std::string>& ForeachNode::condition() const {
    return condition_;
}

const template_expression::ExpressionPtr& ForeachNode::conditionExpression() const {
    return conditionExpression_;
}

std::vector<NodePtr>& ForeachNode::body() {
    return body_;
}

const std::vector<NodePtr>& ForeachNode::body() const {
    return body_;
}

std::vector<NodePtr>& ForeachNode::emptyBranch() {
    return emptyBranch_;
}

const std::vector<NodePtr>& ForeachNode::emptyBranch() const {
    return emptyBranch_;
}

NodeType BreakNode::type() const {
    return NodeType::Break;
}

NodeType ContinueNode::type() const {
    return NodeType::Continue;
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