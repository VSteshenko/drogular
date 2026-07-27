#include "todo_page.hpp"
#include "todo_query_parser.hpp"
#include "todo_query_serializer.hpp"
#include "todo_store.hpp"

#include <drogular/page_support.hpp>
#include <drogular/pwa_scripts.hpp>
#include <drogular/render_context.hpp>
#include <drogular/pagination_model.hpp>

#include <json/json.h>

void TodoPage::onInit(drogular::RenderContext& context) {
    const auto store =
        context.service<TodoStore>();
    auto query =
        TodoQueryParser::fromRequest(context.request());
    const auto result =
        store->find(query);

    query.page = result.page;

    Json::Value todos(Json::arrayValue);
    for (const auto& sourceTodo : result.items) {
        Json::Value todo;

        todo["id"] = sourceTodo.id;
        todo["title"] = sourceTodo.title;
        todo["done"] = sourceTodo.done;

        todos.append(std::move(todo));
    }

    const auto pageUrl = [&query](int page) {
        auto pageQuery = query;
        pageQuery.page = page;
        return std::string("/") + TodoQuerySerializer::toQueryString(pageQuery);
    };

    context.set("title", std::string("Drogular Todo PWA"));
    context.set(
        "subtitle",
        std::string("Angular-inspired C++ web framework for Drogon.")
    );
    context.set("search", query.search);
    context.set("todos", std::move(todos));
    context.set("hasTodos", !result.items.empty());
    context.set("hasSearch", !query.search.empty());
    context.set("totalItems", result.totalItems);
    context.set("page", result.page);
    context.set("totalPages", result.totalPages);
    context.setJson(
        "pagination",
        drogular::makePaginationModel(
            result.page,
            result.totalPages, pageUrl
        )
    );

    context.set(
        "todoDraftPersistence",
        drogular::PwaScripts::inputPersistence(
            "todo-title",
            "todo-draft"
        )
    );
    context.set(
        "offlineStatusScript",
        drogular::PwaScripts::offlineStatus("offline-status")
    );

    drogular::PageSupport::apply(context, "Drogular Todo PWA");
}