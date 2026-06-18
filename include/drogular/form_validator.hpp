#pragma once

#include <drogular/action_context.hpp>
#include <drogular/validation_result.hpp>

#include <string>
#include <vector>

namespace drogular {

class FormValidator {
public:
    explicit FormValidator(
        const ActionContext& context
    );

    /**
     * Requires a non-empty form value.
     */
    FormValidator& required(
        std::string field
    );

    /**
     * Requires a form value to have at least
     * the given number of characters.
     */
    FormValidator& minLength(
        std::string field,
        size_t length
    );

    /**
     * Requires a form value to have at most
     * the given number of characters.
     */
    FormValidator& maxLength(
        std::string field,
        size_t length
    );

    /**
     * Requires a form value to look like an email address.
     */
    FormValidator& email(
        std::string field
    );

    /**
     * Executes validation rules.
     */
    ValidationResult validate() const;

private:
    enum class RuleType {
        Required,
        MinLength,
        MaxLength,
        Email
    };

    struct Rule {
        RuleType type;
        std::string field;
        size_t length = 0;
    };

    static std::string requiredMessage(const std::string& field);

    static std::string minLengthMessage(
        const std::string& field,
        size_t length
    );

    static std::string maxLengthMessage(
        const std::string& field,
        size_t length
    );

    static std::string emailMessage(const std::string& field);

    const ActionContext& context_;
    std::vector<Rule> rules_;
};

} // namespace drogular
