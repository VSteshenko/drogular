#include <drogular/template_source_cache.hpp>

#include <mutex>
#include <shared_mutex>

namespace drogular {

TemplateSourceCache::TemplateSourceCache(
    TemplateLoader loader
)
    : loader_(std::move(loader)) {
}

std::string TemplateSourceCache::load(
    const std::string& path
) {
    {
        std::shared_lock lock(mutex_);

        const auto found =
            cache_.find(path);

        if (found != cache_.end()) {
            return found->second;
        }
    }

    std::unique_lock lock(mutex_);

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
    std::unique_lock lock(mutex_);
    cache_.clear();
}

bool TemplateSourceCache::contains(
    const std::string& path
) const {
    std::shared_lock lock(mutex_);
    return cache_.contains(path);
}

void TemplateSourceCache::setLoader(
    TemplateLoader loader
) {
    std::unique_lock lock(mutex_);
    loader_ = std::move(loader);
    cache_.clear();
}

} // namespace drogular