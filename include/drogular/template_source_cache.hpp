#pragma once

#include <drogular/template_loader.hpp>

#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace drogular {

class TemplateSourceCache {
public:
    explicit TemplateSourceCache(
        TemplateLoader loader = TemplateLoader()
    );

    std::string load(
        const std::string& path
    );

    void clear();

    bool contains(
        const std::string& path
    ) const;

    void setLoader(TemplateLoader loader);

private:
    mutable std::shared_mutex mutex_;
    TemplateLoader loader_;

    std::unordered_map<
        std::string,
        std::string
    > cache_;
};

} // namespace drogular