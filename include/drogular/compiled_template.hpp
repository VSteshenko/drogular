#pragma once

#include <drogular/template_ast.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace drogular {
class RenderContext;
}

namespace drogular::template_compiler {

class CompiledTemplate {
public:
    explicit CompiledTemplate(std::vector<NodePtr> nodes);

    /**
     * Renders the compiled template.
     */
    std::string render(RenderContext& context) const;

private:
    std::vector<NodePtr> nodes_;
};

/**
 * Compiles template text into a reusable template object.
 */
CompiledTemplate compile(std::string_view html);

} // namespace drogular::template_compiler
