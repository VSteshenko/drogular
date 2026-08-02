#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace drogular {

enum class DiagnosticSeverity {
    Info,
    Warning,
    Error
};

struct SourceLocation {
    std::string source;
    std::size_t position = 0;
    std::size_t line = 1;
    std::size_t column = 1;
};

struct Diagnostic {
    std::string code;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string message;
    SourceLocation location;
};

class Diagnostics {
public:
    Diagnostics() = default;

    explicit Diagnostics(
        std::string_view sourceText,
        std::string sourceName = {}
    );

    void add(Diagnostic diagnostic);

    void info(
        std::string code,
        std::string message,
        SourceLocation location = {}
    );

    void warning(
        std::string code,
        std::string message,
        SourceLocation location = {}
    );

    void error(
        std::string code,
        std::string message,
        SourceLocation location = {}
    );

    void info(
        std::string code,
        std::string message,
        std::size_t position
    );

    void warning(
        std::string code,
        std::string message,
        std::size_t position
    );

    void error(
        std::string code,
        std::string message,
        std::size_t position
    );

    void clear();

    bool hasErrors() const;
    bool empty() const;
    bool valid() const;

    const std::vector<Diagnostic>& entries() const;
    const std::vector<Diagnostic>& errors() const;

    SourceLocation locationAt(std::size_t position) const;

private:
    void addAtPosition(
        DiagnosticSeverity severity,
        std::string code,
        std::string message,
        std::size_t position
    );

    std::string sourceText_;
    std::string sourceName_;
    std::vector<Diagnostic> entries_;
    std::vector<Diagnostic> errors_;
};

} // namespace drogular