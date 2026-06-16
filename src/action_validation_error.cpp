#include <drogular/action_validation_error.hpp>

#include <utility>

namespace drogular {

ActionValidationError::ActionValidationError(std::string message)
    : std::runtime_error(std::move(message)) {
}

} // namespace drogular
