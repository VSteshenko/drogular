#include <drogular/template_expression.hpp>

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
        if (std::isdigit(static_cast<unsigned char>(ch)) ||
            (ch == '-' && position_ + 1 < source_.size() &&
             std::isdigit(static_cast<unsigned char>(source_[position_ + 1])))) {
            return numberToken();
        }

        if (isIdentifierCharacter(ch)) {
            return identifierToken();
        }

        ++position_;
        switch (ch) {
            case '(':
                return simple(TokenType::LeftParen, "(", start);

            case ')':
                return simple(TokenType::RightParen, ")", start);

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
    static bool isIdentifierCharacter(char ch) {
        return std::isalnum(static_cast<unsigned char>(ch)) ||
            ch == '_' || ch == '.';
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
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += escaped; break;
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
        if (source_[position_] == '-') {
            ++position_;
        }

        while (position_ < source_.size() &&
               std::isdigit(static_cast<unsigned char>(source_[position_]))) {
            ++position_;
        }

        if (position_ < source_.size() && source_[position_] == '.') {
            ++position_;
            const auto fractionalStart = position_;
            while (position_ < source_.size() &&
                   std::isdigit(static_cast<unsigned char>(source_[position_]))) {
                ++position_;
            }
            if (fractionalStart == position_) {
                error_ = ExpressionError{
                    .message = "Expected digits after decimal point",
                    .position = position_
                };
                return Token{
                    .type = TokenType::Invalid,
                    .position = start
                };
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

ExpressionPtr binary(
    BinaryOperator op,
    ExpressionPtr left,
    ExpressionPtr right
) {
    return std::make_shared<Expression>(Expression{
        .node = BinaryExpression{
            .op = op,
            .left = std::move(left),
            .right = std::move(right)
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
            const auto operatorToken = current_;
            advance();
            auto right = parseAnd();
            if (!right) {
                if (!error_.has_value()) {
                    fail("Expected expression after '||'", operatorToken.position + 2);
                }
                return nullptr;
            }
            left = binary(BinaryOperator::Or, std::move(left), std::move(right));
        }

        return left;
    }

    ExpressionPtr parseAnd() {
        auto left = parseUnary();
        if (!left) {
            return nullptr;
        }

        while (current_.type == TokenType::AndAnd) {
            const auto operatorToken = current_;
            advance();
            auto right = parseUnary();
            if (!right) {
                if (!error_.has_value()) {
                    fail("Expected expression after '&&'", operatorToken.position + 2);
                }
                return nullptr;
            }
            left = binary(BinaryOperator::And, std::move(left), std::move(right));
        }

        return left;
    }

    ExpressionPtr parseUnary() {
        if (current_.type == TokenType::Not) {
            const auto operatorToken = current_;
            advance();
            auto operand = parseUnary();
            if (!operand) {
                if (!error_.has_value()) {
                    fail("Expected expression after '!'", operatorToken.position + 1);
                }
                return nullptr;
            }
            return unary(UnaryOperator::Not, std::move(operand));
        }

        if (current_.type == TokenType::LeftParen) {
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

        return parseComparison();
    }

    ExpressionPtr parseComparison() {
        auto left = parseValue(true);
        if (!left) {
            return nullptr;
        }

        const auto op = binaryOperator(current_.type);
        if (!op.has_value()) {
            return left;
        }

        const auto operatorToken = current_;
        advance();
        auto right = parseValue(false);
        if (!right) {
            if (!error_.has_value()) {
                fail(
                    "Expected value after '" + operatorToken.text + "'",
                    operatorToken.position + operatorToken.text.size()
                );
            }
            return nullptr;
        }

        return binary(*op, std::move(left), std::move(right));
    }

    ExpressionPtr parseValue(bool reportMissing) {
        if (lexer_.error().has_value()) {
            if (!error_.has_value()) {
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

            default:
                if (reportMissing) {
                    fail("Expected value", current_.position);
                }
                return nullptr;
        }
    }

    static std::optional<BinaryOperator> binaryOperator(TokenType type) {
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
        if (current_.type == TokenType::Invalid &&
            lexer_.error().has_value() && !error_.has_value()) {
            error_ = lexer_.error();
        }
    }

    void fail(std::string message, std::size_t position) {
        if (!error_.has_value()) {
            error_ = ExpressionError{
                .message = std::move(message),
                .position = position
            };
        }
    }

    ParseResult failure(std::string message, std::size_t position) {
        fail(std::move(message), position);
        return { .error = error_ };
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