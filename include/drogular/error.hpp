#pragma once

#include <stdexcept>
#include <string>

namespace drogular {

/**
 * Base class for errors reported by Drogular framework APIs.
 */
class DrogularError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

} // namespace drogular