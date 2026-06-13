#pragma once

#include <string>
#include <vector>

namespace drogular::template_compiler {

struct TemplateError {
    std::string message;
    size_t position = 0;
};

class TemplateDiagnostics {
public:
    void addError(std::string message, size_t position);

    bool valid() const;

    const std::vector<TemplateError>& errors() const;

private:
    std::vector<TemplateError> errors_;
};

} // namespace drogular::template_compiler
