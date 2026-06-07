#include <drogular/app.hpp>
#include <drogular/page.hpp>

#include <optional>
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
    drogular::App app;

    app.page<HomePage>("/");
    app.run(8080);
}
