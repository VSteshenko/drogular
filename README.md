# Drogular

Drogular is an Angular-inspired C++ web framework built on top of Drogon.

The goal is to provide a component-oriented way to build SSR, SPA, and PWA-style applications in modern C++ with first-class GraphQL support.

## Status

Experimental. The project is currently in the first prototype stage.

Current features:

- CMake project skeleton
- Drogon integration
- Page and component base classes
- Simple routing through `drogular::App`
- GraphQL query builder
    - Nested selections
    - Typed values
    - Arguments
    - Variables
    - Aliases
    - Fragments
    - Basic validation
- GoogleTest-based test infrastructure
- Todo PWA example
- GitHub Actions CI for Ubuntu and macOS

## GraphQL builder

Drogular includes a small GraphQL query builder.

```cpp
const auto userFields = drogular::gql::fragment("UserFields", "User", {
    drogular::gql::field("id"),
    drogular::gql::field("name")
});

const auto query = drogular::gql::query("UserPage")
    .variable("userId", "ID!")
    .select(
        drogular::gql::field("user")
            .alias("profile")
            .arg("id", drogular::gql::variable("userId"))
            .children({
                drogular::gql::spread("UserFields")
            })
    )
    .fragment(userFields);

const auto validation = query.validate();

if (validation.valid()) {
    const auto text = query.toString();
}

```

## Example

```cpp
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
            .select("viewer", {
                drogular::gql::field("id"), 
                drogular::gql::field("name")
             });
    }

    std::string render(drogular::RenderContext&) override {
        return "<h1>" + title_ + "</h1>";
    }

private:
    std::string title_;
};

int main() {
    drogular::App app;

    app.page<HomePage>("/");
    app.run(8080);
}

```