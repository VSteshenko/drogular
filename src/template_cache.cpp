#include <drogular/template_cache.hpp>

namespace drogular::template_compiler {

std::shared_ptr<CompiledTemplate> TemplateCache::getOrCompile(
    std::string_view html
) {
    const auto key = std::string(html);

    const auto it = templates_.find(key);

    if (it != templates_.end()) {
        return it->second;
    }

    auto compiled =
        std::make_shared<CompiledTemplate>(
            compile(html)
        );

    templates_[key] = compiled;

    return compiled;
}

bool TemplateCache::contains(std::string_view html) const {
    return templates_.contains(std::string(html));
}

void TemplateCache::clear() {
    templates_.clear();
}

} // namespace drogular::template_compiler