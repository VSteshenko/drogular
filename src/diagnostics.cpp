#include <drogular/diagnostics.hpp>

#include <algorithm>
#include <utility>

namespace drogular {

Diagnostics::Diagnostics(
    std::string_view sourceText,
    std::string sourceName
)
    : sourceText_(sourceText),
      sourceName_(std::move(sourceName)) {
}

void Diagnostics::add(Diagnostic diagnostic) {
    if (diagnostic.severity == DiagnosticSeverity::Error) {
        errors_.push_back(diagnostic);
    }

    entries_.push_back(std::move(diagnostic));
}

void Diagnostics::info(
    std::string code,
    std::string message,
    SourceLocation location
) {
    add({
        .code = std::move(code),
        .severity = DiagnosticSeverity::Info,
        .message = std::move(message),
        .location = std::move(location)
    });
}

void Diagnostics::warning(
    std::string code,
    std::string message,
    SourceLocation location
) {
    add({
        .code = std::move(code),
        .severity = DiagnosticSeverity::Warning,
        .message = std::move(message),
        .location = std::move(location)
    });
}

void Diagnostics::error(
    std::string code,
    std::string message,
    SourceLocation location
) {
    add({
        .code = std::move(code),
        .severity = DiagnosticSeverity::Error,
        .message = std::move(message),
        .location = std::move(location)
    });
}

void Diagnostics::info(
    std::string code,
    std::string message,
    std::size_t position
) {
    addAtPosition(
        DiagnosticSeverity::Info,
        std::move(code),
        std::move(message),
        position
    );
}

void Diagnostics::warning(
    std::string code,
    std::string message,
    std::size_t position
) {
    addAtPosition(
        DiagnosticSeverity::Warning,
        std::move(code),
        std::move(message),
        position
    );
}

void Diagnostics::error(
    std::string code,
    std::string message,
    std::size_t position
) {
    addAtPosition(
        DiagnosticSeverity::Error,
        std::move(code),
        std::move(message),
        position
    );
}

bool Diagnostics::hasErrors() const {
    return !errors_.empty();
}

bool Diagnostics::empty() const {
    return entries_.empty();
}

bool Diagnostics::valid() const {
    return !hasErrors();
}

const std::vector<Diagnostic>& Diagnostics::entries() const {
    return entries_;
}

const std::vector<Diagnostic>& Diagnostics::errors() const {
    return errors_;
}

SourceLocation Diagnostics::locationAt(std::size_t position) const {
    position = std::min(position, sourceText_.size());

    SourceLocation location{
        .source = sourceName_,
        .position = position,
        .line = 1,
        .column = 1
    };

    for (std::size_t index = 0; index < position; ++index) {
        if (sourceText_[index] == '\n') {
            ++location.line;
            location.column = 1;
        } else {
            ++location.column;
        }
    }

    return location;
}

void Diagnostics::addAtPosition(
    DiagnosticSeverity severity,
    std::string code,
    std::string message,
    std::size_t position
) {
    add({
        .code = std::move(code),
        .severity = severity,
        .message = std::move(message),
        .location = locationAt(position)
    });
}

} // namespace drogular