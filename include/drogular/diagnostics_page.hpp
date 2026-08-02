#pragma once

#include <drogular/page.hpp>

namespace drogular {

/**
 * Built-in developer diagnostics application shell.
 *
 * The page deliberately consumes the public inspection JSON endpoint from
 * the browser instead of accessing ApplicationInspection directly.
 */
class DiagnosticsPage final : public Page {
public:
    static constexpr const char* Path = "/__drogular";
    static constexpr const char* AssetsPath = "/__drogular/assets";

    std::string render(RenderContext& context) override;
};

} // namespace drogular