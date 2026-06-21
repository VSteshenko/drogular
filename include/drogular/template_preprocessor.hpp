#pragma once

#include <drogular/template_loader.hpp>

#include <string>

namespace drogular {

class TemplatePreprocessor {
public:
    explicit TemplatePreprocessor(
        TemplateLoader loader = TemplateLoader()
    );

    std::string process(
        const std::string& source
    ) const;

private:
    TemplateLoader loader_;

    std::string processIncludes(
        const std::string& source
    ) const;
};

} // namespace drogular