#pragma once

#include <stdexcept>
#include <string>

namespace drogular {

/**
 * Thrown when action input validation fails.
 */
class ActionValidationError : public std::runtime_error {
public:
    explicit ActionValidationError(std::string message);
};

} // namespace drogular
