#include <drogular/pwa_html.hpp>

namespace drogular {

std::string PwaHtml::manifestLink(
    const std::string& path
) {
    return R"(<link rel="manifest" href=")" +
           path +
           R"(">)";
}

std::string PwaHtml::favicon(
    const std::string& path,
    const std::string& type
) {
    return R"(<link rel="icon" type=")" +
           type +
           R"(" href=")" +
           path +
           R"(">)";
}

std::string PwaHtml::themeColor(
    const std::string& color
) {
    return R"(<meta name="theme-color" content=")" +
           color +
           R"(">)";
}

} // namespace drogular