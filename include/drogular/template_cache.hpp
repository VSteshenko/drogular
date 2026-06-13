#pragma once

#include <drogular/compiled_template.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace drogular::template_compiler {

/**
 * Caches compiled templates by template text.
 */
class TemplateCache {
public:
    /**
     * Returns a compiled template from cache or compiles it.
     */
    std::shared_ptr<CompiledTemplate> getOrCompile(
        std::string_view html
    );

    /**
     * Returns true when the template is already cached.
     */
    bool contains(std::string_view html) const;

    /**
     * Clears all cached templates.
     */
    void clear();

private:
    std::unordered_map<
        std::string,
        std::shared_ptr<CompiledTemplate>
    > templates_;
};

} // namespace drogular::template_compiler