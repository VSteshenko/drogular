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

private:
    std::vector<ValidationError> errors_;
};

} // namespace drogular
