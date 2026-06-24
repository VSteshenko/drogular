#include <drogular/pwa_html.hpp>

#include <gtest/gtest.h>

TEST(CorePwaHtmlTests, CreatesManifestLink) {
    EXPECT_EQ(
        drogular::PwaHtml::manifestLink(),
        R"(<link rel="manifest" href="/assets/manifest.webmanifest">)"
    );
}

TEST(CorePwaHtmlTests, CreatesFaviconLink) {
    EXPECT_EQ(
        drogular::PwaHtml::favicon(),
        R"(<link rel="icon" type="image/x-icon" href="/assets/favicon.ico">)"
    );
}

TEST(CorePwaHtmlTests, CreatesThemeColorMeta) {
    EXPECT_EQ(
        drogular::PwaHtml::themeColor("#4f46e5"),
        R"(<meta name="theme-color" content="#4f46e5">)"
    );
}