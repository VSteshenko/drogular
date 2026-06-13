#include <drogular/template_diagnostics.hpp>

#include <string>

namespace drogular::template_compiler {

namespace {

std::string withPosition(
    const std::string& message,
    size_t position
) {
    return message + " at position " + std::to_string(position);
}

}

void TemplateDiagnostics::addError(
    std::string message,
    size_t position
) {
    errors_.push_back({
        .message = withPosition(message, position),
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
