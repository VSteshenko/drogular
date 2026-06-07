#include <drogular/app.hpp>
#include <drogular/page.hpp>
#include <drogular/router.hpp>

#include <memory>
#include <string>

class HomePage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext&) override {
        title_ = "Hello from Drogular";
    }

    std::optional<drogular::gql::Query> query() const override {
        return drogular::gql::query("HomePage")
            .select("viewer", {"id", "name"});
    }

    std::string render(drogular::RenderContext&) override {
        return R"(
<!doctype html>
<html>
<head>
    <title>Drogular Todo PWA</title>
</head>
<body>
    <h1>)" + title_ + R"(</h1>
</body>
</html>
)";
    }

private:
    std::string title_;
};

int main() {
    drogular::Router router;

    router.page("/", std::make_shared<HomePage>());

    drogular::App app;
    app.run(8080);
}
