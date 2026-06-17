#include <drogular/form_validator.hpp>

namespace drogular {

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
                        rule.field + " is required"
                    );
                }
                break;

            case RuleType::MinLength:
                if (!value.has_value() ||
                    value->size() < rule.length) {
                    result.addError(
                        rule.field,
                        rule.field + " is too short"
                    );
                }
                break;
        }
    }

    return result;
}

} // namespace drogular
