#include <drogular/form_validator.hpp>

namespace drogular {

namespace {

bool looksLikeEmail(const std::string& value) {
    const auto at = value.find('@');

    if (at == std::string::npos ||
        at == 0 ||
        at + 1 >= value.size()) {
        return false;
        }

    const auto dot =
        value.find('.', at + 1);

    if (dot == std::string::npos ||
        dot + 1 >= value.size()) {
        return false;
        }

    return true;
}

} // namespace

FormValidator::FormValidator(
    const ActionContext& context
)
    : context_(context) {
}

FormValidator& FormValidator::required(
    std::string field
) {
    rules_.push_back({
        .type = RuleType::Required,
        .field = std::move(field),
        .length = 0
    });

    return *this;
}

FormValidator& FormValidator::minLength(
    std::string field,
    size_t length
) {
    rules_.push_back({
        .type = RuleType::MinLength,
        .field = std::move(field),
        .length = length
    });

    return *this;
}

FormValidator& FormValidator::maxLength(
    std::string field,
    size_t length
) {
    rules_.push_back({
        .type = RuleType::MaxLength,
        .field = std::move(field),
        .length = length
    });

    return *this;
}

FormValidator& FormValidator::email(
    std::string field
) {
    rules_.push_back({
        .type = RuleType::Email,
        .field = std::move(field),
        .length = 0
    });

    return *this;
}

ValidationResult FormValidator::validate() const {
    ValidationResult result;

    for (const auto& rule : rules_) {
        const auto value =
            context_.formValue(rule.field);

        switch (rule.type) {
            case RuleType::Required:
                if (!value.has_value() ||
                    value->empty()) {
                    result.addError(
                        rule.field,
                        requiredMessage(rule.field)
                    );
                }
                break;

            case RuleType::MinLength:
                if (!value.has_value() ||
                    value->size() < rule.length) {
                    result.addError(
                        rule.field,
                        minLengthMessage(rule.field, rule.length)
                    );
                }
                break;

            case RuleType::MaxLength:
                if (value.has_value() &&
                    value->size() > rule.length) {
                    result.addError(
                        rule.field,
                        maxLengthMessage(rule.field, rule.length)
                    );
                }
                break;

            case RuleType::Email:
                if (value.has_value() &&
                    !value->empty() &&
                    !looksLikeEmail(*value)) {
                    result.addError(
                        rule.field,
                        emailMessage(rule.field)
                    );
                }
            break;
        }
    }

    return result;
}

std::string FormValidator::requiredMessage(
    const std::string& field
) {
    return field + " is required";
}

std::string FormValidator::minLengthMessage(
    const std::string& field,
    size_t length
) {
    return field +
           " must be at least " +
           std::to_string(length) +
           " characters";
}

std::string FormValidator::maxLengthMessage(
    const std::string& field,
    size_t length
) {
    return field +
           " must be at most " +
           std::to_string(length) +
           " characters";
}

std::string FormValidator::emailMessage(
    const std::string& field
) {
    return field + " is not a valid email";
}

} // namespace drogular
