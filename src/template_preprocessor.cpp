#include <drogular/template_preprocessor.hpp>

#include <regex>
#include <stdexcept>

namespace drogular {

TemplatePreprocessor::TemplatePreprocessor(
    TemplateLoader loader
)
    : loader_(std::move(loader)) {
}

std::string TemplatePreprocessor::process(
    const std::string& source
) const {
    auto current = source;

    constexpr int maxIncludeDepth = 16;

    for (int depth = 0; depth < maxIncludeDepth; ++depth) {
        if (current.find("@include(") == std::string::npos) {
            return current;
        }

        const auto next = processIncludes(current);

        if (next == current) {
            return current;
        }

        current = next;
    }

    throw std::runtime_error(
        "Maximum template include depth exceeded"
    );
}

std::string TemplatePreprocessor::processIncludes(
    const std::string& source
) const {
    static const std::regex includePattern(
        R"regex(@include\("([^"]+)"\))regex"
    );

    std::string result;
    std::sregex_iterator current(
        source.begin(),
        source.end(),
        includePattern
    );
    std::sregex_iterator end;

    size_t last = 0;

    for (; current != end; ++current) {
        const auto& match = *current;

        result.append(
            source,
            last,
            match.position() - last
        );

        const auto path =
            match[1].str();

        result += loader_.load(path);

        last =
            match.position() + match.length();
    }

    result.append(
        source,
        last,
        std::string::npos
    );

    return result;
}

} // namespace drogular