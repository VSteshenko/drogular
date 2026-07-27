#pragma once

#include <drogular/application_options.hpp>
#include <drogular/services.hpp>

#include <filesystem>

inline void configureAuthSampleServices(
    drogular::ApplicationServices& services,
    drogular::ApplicationOptions& options
) {
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR)
        / "examples/auth_sample/templates"
    );

    services.setOptions(&options);
}
