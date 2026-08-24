#pragma once

#include <drogular/template_ast.hpp>
#include <drogular/template_diagnostics.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace drogular {
class RenderContext;
namespace template_expression { class BindingContext; }
}

namespace drogular::template_compiler {

class CompiledTemplate {
public:
    explicit CompiledTemplate(std::vector<NodePtr> nodes);

    /**
     * Renders the compiled template.
     */
    std::string render(RenderContext& context) const;

    /**
     * Renders using an existing lexical binding scope.
     *
     * The binding context must use the supplied RenderContext (or one of its
     * parents) as its fallback data source. This overload is primarily used
     * by template/component integration so lexical bindings do not need to be
     * materialized merely to evaluate nested template fragments.
     */
    std::string render(
        RenderContext& context,
        template_expression::BindingContext& bindings
    ) const;

private:
    std::vector<NodePtr> nodes_;
};

/**
 * Compiles template text into a reusable template object.
 */
CompiledTemplate compile(std::string_view html);

struct CompileResult {
    CompiledTemplate compiledTemplate;
    TemplateDiagnostics diagnostics;

    bool valid() const {
        return diagnostics.valid();
    }
};

/**
 * Compiles template text and returns diagnostics.
 */
CompileResult compileWithDiagnostics(
    std::string_view html,
    std::string sourceName = {}
);

} // namespace drogular::template_compiler