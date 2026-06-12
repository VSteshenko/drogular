#pragma once

#include <drogular/services.hpp>

#include <memory>

namespace drogular {

template <typename ServiceType, typename... Dependencies>
void registerDependencies(ApplicationServices& services) {
    (
        services.dependencyGraph()
            .addDependency<ServiceType, Dependencies>(),
        ...
    );
}

/**
 * Creates a factory that resolves dependencies from ApplicationServices
 * and passes them into the target constructor.
 */
template <typename ServiceType, typename... Dependencies>
auto inject(ApplicationServices& services) {
    registerDependencies<ServiceType, Dependencies...>(services);

    return [&services]() {
        return std::make_shared<ServiceType>(
            services.requireService<Dependencies>()...
        );
    };
}

} // namespace drogular
