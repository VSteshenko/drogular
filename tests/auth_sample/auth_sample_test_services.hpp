#pragma once

#include <drogular/application_options.hpp>
#include <drogular/services.hpp>

inline void configureAuthSampleServices(
    drogular::ApplicationServices& services,
    drogular::ApplicationOptions& options
) {
    options.setTemplateRoot(
        "../../examples/auth_sample/templates"
    );

    services.setOptions(&options);
}
