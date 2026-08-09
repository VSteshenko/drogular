#pragma once

#include <drogular/error.hpp>
#include <string>

namespace drogular {

/**
 * Thrown when action input validation fails.
 */
class ActionValidationError : public DrogularError {
public:
    explicit ActionValidationError(std::string message);
};

} // namespace drogular
