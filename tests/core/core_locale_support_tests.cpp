#include <drogular/locale_support.hpp>

#include <gtest/gtest.h>

TEST(CoreLocaleSupportTests, DefaultsToEnglish) {
    drogular::RenderContext context;

    EXPECT_EQ(
        drogular::LocaleSupport::current(context),
        "en"
    );
}

TEST(CoreLocaleSupportTests, UsesCustomDefaultLocale) {
    drogular::RenderContext context;

    EXPECT_EQ(
        drogular::LocaleSupport::current(context, "de"),
        "de"
    );
}