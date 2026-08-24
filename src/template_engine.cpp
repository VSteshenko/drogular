#include <drogular/template_engine.hpp>
#include <drogular/compiled_template.hpp>
#include <drogular/render_context.hpp>

namespace drogular::template_engine {

std::string render(
    std::string_view html,
    const RenderContext& context
) {
    auto result = template_compiler::compileWithDiagnostics(html);
    if (result.diagnostics.hasErrors()) {
        return std::string(html);
    }

    auto renderContext = context.createChild();
    return result.compiledTemplate.render(renderContext);
}

} // namespace drogular::template_engine