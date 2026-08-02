#include <drogular/diagnostics_page.hpp>
#include <drogular/diagnostics_resources.hpp>

namespace drogular {

std::string DiagnosticsPage::render(RenderContext&) {
    return std::string(diagnostics_resources::pageHtml());
}

} // namespace drogular