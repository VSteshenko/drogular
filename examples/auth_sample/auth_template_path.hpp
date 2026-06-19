#pragma once

#include <filesystem>
#include <iostream>
#include <string>

inline std::string authTemplatePath(
    const std::string& name
) {
    const auto fromProjectRoot =
        std::filesystem::path(
            "examples/auth_sample/templates"
        ) / name;

    std::cout << "Check current path: " << std::filesystem::current_path().string() << std::endl;

    if (std::filesystem::exists(fromProjectRoot)) {
        return fromProjectRoot.string();
    }

    const auto fromBuildRoot =
        std::filesystem::path(
            "../../examples/auth_sample/templates"
        ) / name;

    if (std::filesystem::exists(fromBuildRoot)) {
        return fromBuildRoot.string();
    }

    return fromProjectRoot.string();
}