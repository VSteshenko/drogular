#include <drogular/template_cache.hpp>

#include <mutex>
#include <shared_mutex>

namespace drogular::template_compiler {

std::shared_ptr<CompiledTemplate> TemplateCache::getOrCompile(
    std::string_view html
) {
    const auto key = std::string(html);

    {
        std::shared_lock lock(mutex_);
        const auto it = templates_.find(key);
        if (it != templates_.end()) {
            return it->second;
        }
    }

    auto compiled =
        std::make_shared<CompiledTemplate>(
            compile(html)
        );

    std::unique_lock lock(mutex_);
    const auto [it, inserted] = templates_.emplace(key, compiled);
    return inserted ? compiled : it->second;
}

bool TemplateCache::contains(std::string_view html) const {
    std::shared_lock lock(mutex_);
    return templates_.contains(std::string(html));
}

void TemplateCache::clear() {
    std::unique_lock lock(mutex_);
    templates_.clear();
}

} // namespace drogular::template_compiler