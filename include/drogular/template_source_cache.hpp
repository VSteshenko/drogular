#pragma once

#include <drogular/template_loader.hpp>

#include <string>
#include <unordered_map>

namespace drogular {

class TemplateSourceCache {
public:
    explicit TemplateSourceCache(
        TemplateLoader loader = TemplateLoader()
    );

    const std::string& load(
        const std::string& path
    );

    void clear();

    bool contains(
        const std::string& path
    ) const;

    void setLoader(TemplateLoader loader);

private:
    TemplateLoader loader_;

    std::unordered_map<
        std::string,
        std::string
    > cache_;
};

} // namespace drogular