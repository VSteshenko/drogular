#include <drogular/app.hpp>
#include <drogular/page.hpp>
#include <drogular/router.hpp>

#include <memory>
#include <string>

class HomePage final : public drogular::Page {
public:
    std::string render() override {
        return R"(
<!doctype html>
<html>
<head>
    <title>Drogular Todo PWA</title>
</head>
<body>
    <h1>Hello from Drogular</h1>
</body>
</html>
)";
    }
};

int main() {
    drogular::Router router;

    router.page("/", std::make_shared<HomePage>());

    drogular::App app;
    app.run(8080);
}
