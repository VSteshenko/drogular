#include <drogular/template_diagnostics.hpp>

#include <utility>

namespace drogular::template_compiler {

void TemplateDiagnostics::addError(
    std::string message,
    size_t position
) {
    errors_.push_back({
        .message = std::move(message),
        .position = position
    });
}

bool TemplateDiagnostics::valid() const {
    return errors_.empty();
}

const std::vector<TemplateError>& TemplateDiagnostics::errors() const {
    return errors_;
}

} // namespace drogular::template_compiler
