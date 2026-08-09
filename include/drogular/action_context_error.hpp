#pragma once

#include <drogular/error.hpp>

#include <string>

namespace drogular {

/**
 * Thrown when required ActionContext infrastructure or request data is unavailable.
 */
class ActionContextError : public DrogularError {
public:
    using DrogularError::DrogularError;
};

} // namespace drogular