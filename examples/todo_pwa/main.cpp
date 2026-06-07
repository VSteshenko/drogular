#include "todo_page.hpp"

#include <drogular/app.hpp>

int main() {
    drogular::App app;

    app.page<TodoPage>("/");
    app.run(8080);
}