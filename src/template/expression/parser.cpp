#include <drogular/template/expression/parser.hpp>

#include <cctype>
#include <string>
#include <utility>

namespace drogular::template_expression {

namespace {

enum class TokenType {
    End,
    Identifier,
    Number,
    String,
    True,
    False,
    Null,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    Comma,
    Plus,
    Minus,
    Star,
    Slash,
    Dot,
    DotDot,
    DotDotLess,
    Not,
    EqualEqual,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    AndAnd,
    OrOr,
    Invalid
};

struct Token {
    TokenType type = TokenType::End;
    std::string text;
    std::size_t position = 0;
    ExpressionValue literal;
};

class Lexer {
public:
    explicit Lexer(std::string_view source) : source_(source) {}

    Token next() {
        skipWhitespace();
        if (position_ >= source_.size()) {
            return Token{ .type = TokenType::End, .position = position_ };
        }

        const auto start = position_;
        const auto ch = source_[position_];

        if (ch == '\'' || ch == '"') {
            return stringToken();
        }
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            return numberToken();
        }
        if (isIdentifierStart(ch)) {
            return identifierToken();
        }

        ++position_;
        switch (ch) {
            case '(':
                return simple(TokenType::LeftParen, "(", start);

            case ')':
                return simple(TokenType::RightParen, ")", start);

            case '[':
                return simple(TokenType::LeftBracket, "[", start);

            case ']':
                return simple(TokenType::RightBracket, "]", start);

            case ',':
                return simple(TokenType::Comma, ",", start);

            case '+':
                return simple(TokenType::Plus, "+", start);

            case '-':
                return simple(TokenType::Minus, "-", start);

            case '*':
                return simple(TokenType::Star, "*", start);

            case '/':
                return simple(TokenType::Slash, "/", start);

            case '.':
                if (match('.')) {
                    if (match('<')) {
                        return simple(TokenType::DotDotLess, "..<", start);
                    }
                    return simple(TokenType::DotDot, "..", start);
                }
                return simple(TokenType::Dot, ".", start);

            case '!':
                if (match('=')) {
                    return simple(TokenType::NotEqual, "!=", start);
                }
                return simple(TokenType::Not, "!", start);

            case '=':
                if (match('=')) {
                    return simple(TokenType::EqualEqual, "==", start);
                }
                break;

            case '<':
                if (match('=')) {
                    return simple(TokenType::LessEqual, "<=", start);
                }
                return simple(TokenType::Less, "<", start);

            case '>':
                if (match('=')) {
                    return simple(TokenType::GreaterEqual, ">=", start);
                }
                return simple(TokenType::Greater, ">", start);

            case '&':
                if (match('&')) {
                    return simple(TokenType::AndAnd, "&&", start);
                }
                break;

            case '|':
                if (match('|')) {
                    return simple(TokenType::OrOr, "||", start);
                }
                break;

            default:
                break;
        }

        return Token{
            .type = TokenType::Invalid,
            .text = std::string(source_.substr(start, position_ - start)),
            .position = start
        };
    }

    [[nodiscard]] const std::optional<ExpressionError>& error() const {
        return error_;
    }

private:
    static bool isIdentifierStart(char ch) {
        return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
    }

    static bool isIdentifierCharacter(char ch) {
        return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
    }

    Token simple(TokenType type, std::string text, std::size_t position) {
        return Token{
            .type = type,
            .text = std::move(text),
            .position = position
        };
    }

    Token stringToken() {
        const auto start = position_;
        const auto quote = source_[position_++];
        std::string result;

        while (position_ < source_.size()) {
            const auto ch = source_[position_++];
            if (ch == quote) {
                return Token{
                    .type = TokenType::String,
                    .text = std::string(source_.substr(start, position_ - start)),
                    .position = start,
                    .literal = ExpressionValue(std::move(result))
                };
            }

            if (ch == '\\' && position_ < source_.size()) {
                const auto escaped = source_[position_++];
                switch (escaped) {
                    case 'n':
                        result += '\n';
                        break;

                    case 'r':
                        result += '\r';
                        break;

                    case 't':
                        result += '\t';
                        break;

                    default:
                        result += escaped;
                        break;
                }
            } else {
                result += ch;
            }
        }

        error_ = ExpressionError{
            .message = "Unterminated string literal",
            .position = start
        };
        return Token{
            .type = TokenType::Invalid,
            .position = start
        };
    }

    Token numberToken() {
        const auto start = position_;
        while (position_ < source_.size() &&
               std::isdigit(static_cast<unsigned char>(source_[position_]))
        ) {
            ++position_;
        }

        // A decimal point is only part of a number when it is followed by a
        // digit. This keeps `1..10` and `1..<10` unambiguous.
        if (position_ + 1 < source_.size() &&
            source_[position_] == '.' &&
            source_[position_ + 1] != '.' &&
            std::isdigit(static_cast<unsigned char>(source_[position_ + 1]))
        ) {
            ++position_;
            while (position_ < source_.size() &&
                   std::isdigit(static_cast<unsigned char>(source_[position_]))) {
                ++position_;
            }
        }

        const auto text = std::string(source_.substr(start, position_ - start));
        try {
            return Token{
                .type = TokenType::Number,
                .text = text,
                .position = start,
                .literal = ExpressionValue(std::stod(text))
            };
        } catch (...) {
            error_ = ExpressionError{
                .message = "Invalid number literal",
                .position = start
            };
            return Token{
                .type = TokenType::Invalid,
                .position = start
            };
        }
    }

    Token identifierToken() {
        const auto start = position_;
        while (position_ < source_.size() &&
               isIdentifierCharacter(source_[position_])) {
            ++position_;
        }

        // Dotted member paths are identifiers, but `..` / `..<` belong to
        // range syntax and must remain separate tokens.
        while (position_ < source_.size() && source_[position_] == '.' &&
               position_ + 1 < source_.size() && source_[position_ + 1] != '.' &&
               isIdentifierStart(source_[position_ + 1])) {
            ++position_;
            while (position_ < source_.size() &&
                   isIdentifierCharacter(source_[position_])) {
                ++position_;
            }
        }

        auto text = std::string(source_.substr(start, position_ - start));
        if (text == "true") {
            return Token{
                .type = TokenType::True,
                .text = std::move(text),
                .position = start,
                .literal = ExpressionValue(true)
            };
        }
        if (text == "false") {
            return Token{
                .type = TokenType::False,
                .text = std::move(text),
                .position = start,
                .literal = ExpressionValue(false)
            };
        }
        if (text == "null") {
            return Token{
                .type = TokenType::Null,
                .text = std::move(text),
                .position = start
            };
        }

        return Token{
            .type = TokenType::Identifier,
            .text = std::move(text),
            .position = start
        };
    }

    bool match(char expected) {
        if (position_ >= source_.size() || source_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    void skipWhitespace() {
        while (position_ < source_.size() &&
               std::isspace(static_cast<unsigned char>(source_[position_]))) {
            ++position_;
        }
    }

    std::string_view source_;
    std::size_t position_ = 0;
    std::optional<ExpressionError> error_;
};

ExpressionPtr literal(ExpressionValue value) {
    return std::make_shared<Expression>(Expression{
        .node = LiteralExpression{ .value = std::move(value) }
    });
}

ExpressionPtr variable(std::string path) {
    return std::make_shared<Expression>(Expression{
        .node = VariableExpression{ .path = std::move(path) }
    });
}

ExpressionPtr unary(UnaryOperator op, ExpressionPtr operand) {
    return std::make_shared<Expression>(Expression{
        .node = UnaryExpression{
            .op = op,
            .operand = std::move(operand)
        }
    });
}

ExpressionPtr binary(BinaryOperator op, ExpressionPtr left, ExpressionPtr right) {
    return std::make_shared<Expression>(Expression{
        .node = BinaryExpression{
            .op = op,
            .left = std::move(left),
            .right = std::move(right)
        }
    });
}

ExpressionPtr member(ExpressionPtr object, std::string name) {
    return std::make_shared<Expression>(Expression{
        .node = MemberAccessExpression{
            .object = std::move(object),
            .member = std::move(name)
        }
    });
}

ExpressionPtr call(
    std::string function,
    std::vector<ExpressionPtr> arguments
) {
    return std::make_shared<Expression>(Expression{
        .node = CallExpression{
            .function = std::move(function),
            .arguments = std::move(arguments)
        }
    });
}

ExpressionPtr methodCall(
    ExpressionPtr object,
    std::string method,
    std::vector<ExpressionPtr> arguments
) {
    return std::make_shared<Expression>(Expression{
        .node = MethodCallExpression{
            .object = std::move(object),
            .method = std::move(method),
            .arguments = std::move(arguments)
        }
    });
}

class Parser {
public:
    explicit Parser(std::string_view source) : lexer_(source) {
        advance();
    }

    ParseResult parseExpression() {
        if (lexer_.error().has_value()) {
            return { .error = lexer_.error() };
        }
        if (current_.type == TokenType::End) {
            return failure("Expected condition expression", current_.position);
        }

        auto expression = parseOr();
        if (!expression) {
            return { .error = error_ };
        }

        if (current_.type != TokenType::End) {
            if (lexer_.error().has_value()) {
                return { .error = lexer_.error() };
            }
            return failure("Unexpected token", current_.position);
        }

        return { .expression = std::move(expression) };
    }

private:
    ExpressionPtr parseOr() {
        auto left = parseAnd();
        if (!left) {
            return nullptr;
        }

        while (current_.type == TokenType::OrOr) {
            const auto op = current_;
            advance();
            auto right = parseAnd();
            if (!right) {
                if (!error_) {
                    fail("Expected expression after '||'", op.position + 2);
                }
                return nullptr;
            }
            left = binary(
                BinaryOperator::Or,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

    ExpressionPtr parseAnd() {
        auto left = parseComparison();
        if (!left) {
            return nullptr;
        }

        while (current_.type == TokenType::AndAnd) {
            const auto op = current_;
            advance();
            auto right = parseComparison();
            if (!right) {
                if (!error_) {
                    fail("Expected expression after '&&'", op.position + 2);
                }
                return nullptr;
            }
            left = binary(
                BinaryOperator::And,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

    ExpressionPtr parseComparison() {
        auto left = parseAdditive();
        if (!left) {
            return nullptr;
        }

        auto op = comparisonOperator(current_.type);
        std::string operatorText;
        std::size_t operatorPosition = current_.position;

        if (current_.type == TokenType::Identifier && current_.text == "in") {
            op = BinaryOperator::In;
            operatorText = "in";
            advance();
        } else if (current_.type == TokenType::Identifier && current_.text == "not") {
            const auto notToken = current_;
            advance();
            if (current_.type != TokenType::Identifier || current_.text != "in") {
                fail("Expected 'in' after 'not'", current_.position);
                return nullptr;
            }
            op = BinaryOperator::NotIn;
            operatorText = "not in";
            operatorPosition = notToken.position;
            advance();
        } else if (op) {
            operatorText = current_.text;
            advance();
        } else {
            return left;
        }

        auto right = parseAdditive();
        if (!right) {
            if (!error_) {
                fail(
                    "Expected value after '" + operatorText + "'",
                    operatorPosition + operatorText.size()
                );
            }
            return nullptr;
        }

        return binary(
            *op,
            std::move(left),
            std::move(right)
        );
    }

    ExpressionPtr parseAdditive() {
        auto left = parseMultiplicative();
        if (!left) {
            return nullptr;
        }

        while (current_.type == TokenType::Plus || current_.type == TokenType::Minus) {
            const auto type = current_.type;
            const auto opToken = current_;
            advance();
            auto right = parseMultiplicative();
            if (!right) {
                if (!error_) {
                    fail(
                        "Expected expression after '" + opToken.text + "'",
                        opToken.position + opToken.text.size()
                    );
                }
                return nullptr;
            }
            left = binary(
                type == TokenType::Plus ? BinaryOperator::Add : BinaryOperator::Subtract,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

    ExpressionPtr parseMultiplicative() {
        auto left = parseUnary();
        if (!left) {
            return nullptr;
        }

        while (current_.type == TokenType::Star || current_.type == TokenType::Slash) {
            const auto type = current_.type;
            const auto opToken = current_;
            advance();
            auto right = parseUnary();
            if (!right) {
                if (!error_) {
                    fail(
                        "Expected expression after '" + opToken.text + "'",
                        opToken.position + opToken.text.size()
                    );
                }
                return nullptr;
            }
            left = binary(
                type == TokenType::Star ? BinaryOperator::Multiply : BinaryOperator::Divide,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

    ExpressionPtr parseUnary() {
        if (current_.type == TokenType::Not || current_.type == TokenType::Minus) {
            const auto token = current_;
            const auto op = current_.type == TokenType::Not
                ? UnaryOperator::Not
                : UnaryOperator::Negate;
            advance();
            auto operand = parseUnary();
            if (!operand) {
                if (!error_) {
                    fail(
                        "Expected expression after '" + token.text + "'",
                        token.position + token.text.size()
                    );
                }
                return nullptr;
            }
            return unary(op, std::move(operand));
        }

        return parsePostfix();
    }

    ExpressionPtr parsePostfix() {
        auto expression = parsePrimary();
        if (!expression) {
            return nullptr;
        }

        // Existing dotted identifiers remain a single VariableExpression for
        // compatibility. When followed by `(`, the last path component is a
        // method and the prefix is its receiver.
        if (current_.type == TokenType::LeftParen) {
            if (const auto* variableNode =
                    std::get_if<VariableExpression>(&expression->node)
            ) {
                const auto separator = variableNode->path.rfind('.');
                auto arguments = parseArguments();
                if (!arguments) {
                    return nullptr;
                }
                if (separator == std::string::npos) {
                    expression =
                        call(variableNode->path, std::move(*arguments));
                } else {
                    expression = methodCall(
                        variable(variableNode->path.substr(0, separator)),
                        variableNode->path.substr(separator + 1),
                        std::move(*arguments)
                    );
                }
            }
        }

        while (current_.type == TokenType::Dot) {
            advance();
            if (current_.type != TokenType::Identifier) {
                fail("Expected member name after '.'", current_.position);
                return nullptr;
            }

            auto path = current_.text;
            advance();

            // The lexer may keep a dotted suffix together (for compatibility
            // with old dotted paths). Split it into member nodes, treating the
            // final component as a method when followed by `(`.
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start <= path.size()) {
                const auto end = path.find('.', start);
                parts.push_back(path.substr(
                    start,
                    end == std::string::npos ? std::string::npos : end - start
                ));
                if (end == std::string::npos) {
                    break;
                }
                start = end + 1;
            }

            const bool methodAtEnd = current_.type == TokenType::LeftParen;
            const auto memberCount = methodAtEnd && !parts.empty()
                ? parts.size() - 1
                : parts.size();
            for (std::size_t index = 0; index < memberCount; ++index) {
                expression = member(std::move(expression), parts[index]);
            }

            if (methodAtEnd) {
                auto arguments = parseArguments();
                if (!arguments) {
                    return nullptr;
                }
                expression = methodCall(
                    std::move(expression),
                    parts.back(),
                    std::move(*arguments)
                );
            }
        }

        return expression;
    }

    std::optional<std::vector<ExpressionPtr>> parseArguments() {
        if (current_.type != TokenType::LeftParen) {
            return std::nullopt;
        }
        advance();

        std::vector<ExpressionPtr> arguments;
        if (current_.type == TokenType::RightParen) {
            advance();
            return arguments;
        }

        while (true) {
            auto argument = parseOr();
            if (!argument) {
                return std::nullopt;
            }
            arguments.push_back(std::move(argument));

            if (current_.type == TokenType::RightParen) {
                advance();
                return arguments;
            }
            if (current_.type != TokenType::Comma) {
                fail(
                    "Expected ',' or ')' in function call",
                    current_.position);
                return std::nullopt;
            }
            advance();
        }
    }

    ExpressionPtr parsePrimary() {
        if (lexer_.error().has_value()) {
            if (!error_) {
                error_ = lexer_.error();
            }
            return nullptr;
        }

        switch (current_.type) {
            case TokenType::String:
            case TokenType::Number:
            case TokenType::True:
            case TokenType::False: {
                auto result = literal(current_.literal);
                advance();
                return result;
            }

            case TokenType::Null: {
                auto result = literal(ExpressionValue());
                advance();
                return result;
            }

            case TokenType::Identifier: {
                auto result = variable(current_.text);
                advance();
                return result;
            }

            case TokenType::LeftParen: {
                advance();
                auto expression = parseOr();
                if (!expression) {
                    return nullptr;
                }
                if (current_.type != TokenType::RightParen) {
                    fail("Expected ')'", current_.position);
                    return nullptr;
                }
                advance();
                return expression;
            }

            case TokenType::LeftBracket:
                return parseBracketExpression();

            default:
                fail("Expected value", current_.position);
                return nullptr;
        }
    }

    ExpressionPtr parseBracketExpression() {
        const auto openPosition = current_.position;
        advance();

        if (current_.type == TokenType::RightBracket) {
            advance();
            return std::make_shared<Expression>(Expression{
                .node = ListExpression{}
            });
        }

        auto first = parseOr();
        if (!first) {
            return nullptr;
        }

        if (current_.type == TokenType::DotDot || current_.type == TokenType::DotDotLess) {
            const bool upperInclusive = current_.type == TokenType::DotDot;
            advance();

            auto end = parseOr();
            if (!end) {
                if (!error_) {
                    fail("Expected range end expression", current_.position);
                }
                return nullptr;
            }

            ExpressionPtr step;
            if (current_.type == TokenType::Identifier && current_.text == "step") {
                const auto stepToken = current_;
                advance();
                step = parseOr();
                if (!step) {
                    if (!error_) {
                        fail("Expected expression after 'step'", stepToken.position + 4);
                    }
                    return nullptr;
                }
            }

            if (current_.type != TokenType::RightBracket) {
                fail("Expected ']' after range expression", current_.position);
                return nullptr;
            }
            advance();

            return std::make_shared<Expression>(Expression{
                .node = RangeExpression{
                    .start = std::move(first),
                    .end = std::move(end),
                    .step = std::move(step),
                    .upperInclusive = upperInclusive
                }
            });
        }

        std::vector<ExpressionPtr> elements;
        elements.push_back(std::move(first));

        while (current_.type == TokenType::Comma) {
            advance();
            if (current_.type == TokenType::RightBracket) {
                // Trailing comma is accepted.
                break;
            }
            auto element = parseOr();
            if (!element) return nullptr;
            elements.push_back(std::move(element));
        }

        if (current_.type != TokenType::RightBracket) {
            fail("Expected ',' or ']' in list literal", current_.position);
            return nullptr;
        }
        advance();

        return std::make_shared<Expression>(Expression{
            .node = ListExpression{ .elements = std::move(elements) }
        });
    }

    static std::optional<BinaryOperator> comparisonOperator(TokenType type) {
        switch (type) {
            case TokenType::EqualEqual:
                return BinaryOperator::Equal;

            case TokenType::NotEqual:
                return BinaryOperator::NotEqual;

            case TokenType::Less:
                return BinaryOperator::Less;

            case TokenType::LessEqual:
                return BinaryOperator::LessEqual;

            case TokenType::Greater:
                return BinaryOperator::Greater;

            case TokenType::GreaterEqual:
                return BinaryOperator::GreaterEqual;

            default:
                return std::nullopt;
        }
    }

    void advance() {
        current_ = lexer_.next();
        if (current_.type == TokenType::Invalid && lexer_.error().has_value() && !error_) {
            error_ = lexer_.error();
        }
    }

    void fail(std::string message, std::size_t position) {
        if (!error_) {
            error_ = ExpressionError{
                .message = std::move(message),
                .position = position
            };
        }
    }

    ParseResult failure(std::string message, std::size_t position) {
        return ParseResult{
            .error = ExpressionError{
                .message = std::move(message),
                .position = position
            }
        };
    }

    Lexer lexer_;
    Token current_;
    std::optional<ExpressionError> error_;
};

} // namespace

ParseResult parse(std::string_view source) {
    return Parser(source).parseExpression();
}

} // namespace drogular::template_expression