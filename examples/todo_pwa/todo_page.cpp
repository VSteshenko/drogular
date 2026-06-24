#include "todo_page.hpp"
#include "todo.hpp"
#include "todo_store.hpp"

#include <drogular/pwa_scripts.hpp>
#include <drogular/pwa_html.hpp>
#include <drogular/pwa_page_support.hpp>

#include <vector>
#include <json/json.h>

void TodoPage::onInit(drogular::RenderContext& context) {
    auto store =
        context.service<TodoStore>();

    const auto& sourceTodos =
        store->todos.value();

    Json::Value todos(Json::arrayValue);

    for (const auto& sourceTodo : sourceTodos) {
        Json::Value todo;

        todo["id"] = sourceTodo.id;
        todo["title"] = sourceTodo.title;
        todo["done"] = sourceTodo.done;

        todos.append(todo);
    }

    context.set("title", std::string("Drogular Todo PWA"));

    context.set(
        "subtitle",
        std::string("Angular-inspired C++ web framework for Drogon.")
    );

    context.set("todos", todos);
    context.set("hasTodos", !sourceTodos.empty());

    drogular::PwaPageSupport::apply(context);
}