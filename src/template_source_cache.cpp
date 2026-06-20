#include <drogular/template_source_cache.hpp>

namespace drogular {

TemplateSourceCache::TemplateSourceCache(
    TemplateLoader loader
)
    : loader_(std::move(loader)) {
}

const std::string& TemplateSourceCache::load(
    const std::string& path
) {
    const auto found =
        cache_.find(path);

    if (found != cache_.end()) {
        return found->second;
    }

    auto loaded =
        loader_.load(path);

    const auto [inserted, _] =
        cache_.emplace(
            path,
            std::move(loaded)
        );

    return inserted->second;
}

void TemplateSourceCache::clear() {
    cache_.clear();
}

bool TemplateSourceCache::contains(
    const std::string& path
) const {
    return cache_.contains(path);
}

} // namespace drogular