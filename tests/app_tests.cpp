#include <drogular/app.hpp>
#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class AppTestPage final : public drogular::Page {
public:
    std::string render(drogular::RenderContext&) override {
        return "<h1>App Test</h1>";
    }
};

TEST(AppTests, AppCanRegisterPageType) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.page<AppTestPage>("/app-test");
    });
}
