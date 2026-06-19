#pragma once

#include <string>

namespace drogular {

class TemplateLoader {
public:
    /**
     * Loads template file content.
     *
     * Throws std::runtime_error when the file cannot be opened.
     */
    std::string load(
        const std::string& path
    ) const;
};

} // namespace drogular