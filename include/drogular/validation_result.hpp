#pragma once

#include <string>
#include <utility>
#include <vector>

namespace drogular {

struct ValidationError {
    std::string field;
    std::string message;
};

/**
 * Stores validation errors.
 */
class ValidationResult {
public:
    /**
     * Adds a validation error.
     */
    void addError(
        std::string field,
        std::string message
    ) {
        errors_.push_back({
            .field = std::move(field),
            .message = std::move(message)
        });
    }

    /**
     * Returns true when validation contains no errors.
     */
    bool valid() const {
        return errors_.empty();
    }

    /**
     * Returns validation errors.
     */
    const std::vector<ValidationError>& errors() const {
        return errors_;
    }

    /**
     * Returns true when the specified field
     * contains at least one validation error.
     */
    bool hasError(
        const std::string& field
    ) const {
        for (const auto& error : errors_) {
            if (error.field == field) {
                return true;
            }
        }

        return false;
    }

    /**
     * Returns the first error message for the
     * specified field.
     */
    std::optional<std::string> error(
        const std::string& field
    ) const {
        for (const auto& error : errors_) {
            if (error.field == field) {
                return error.message;
            }
        }

        return std::nullopt;
    }

private:
    std::vector<ValidationError> errors_;
};

} // namespace drogular
