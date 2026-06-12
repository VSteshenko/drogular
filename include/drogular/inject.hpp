#pragma once

#include <drogular/services.hpp>

#include <memory>

namespace drogular {

/**
 * Creates a factory that resolves dependencies from ApplicationServices
 * and passes them into the target constructor.
 */
template <typename ServiceType, typename... Dependencies>
auto inject(ApplicationServices& services) {
    return [&services]() {
        return std::make_shared<ServiceType>(
            services.requireService<Dependencies>()...
        );
    };
}

} // namespace drogular
