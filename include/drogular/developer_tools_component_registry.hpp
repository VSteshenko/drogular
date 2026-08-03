#pragma once

#include <json/json.h>

#include <string>
#include <string_view>
#include <vector>

namespace drogular {

struct DeveloperToolsComponentRegistration {
    std::string name;
    std::string module;
};

/**
 * Registry of trusted browser-side renderers used by DiagnosticsPage.
 *
 * A renderer module must export either `render(container, context)` or a
 * default function with the same signature. The context contains the full
 * section metadata and its inspection data.
 */
class DeveloperToolsComponentRegistry {
public:
    static constexpr const char* Path =
        "/__drogular/developer-tools/components";

    void add(std::string name, std::string module);

    const DeveloperToolsComponentRegistration* find(
        std::string_view name
    ) const;

    const std::vector<DeveloperToolsComponentRegistration>& entries() const;

    Json::Value toJson() const;

private:
    std::vector<DeveloperToolsComponentRegistration> entries_;
};

} // namespace drogular