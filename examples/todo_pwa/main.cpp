#include "todo_page.hpp"
#include "todo_item_component.hpp"
#include "create_todo_action.hpp"
#include "todo_store.hpp"
#include "toggle_todo_action.hpp"
#include "delete_todo_action.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <memory>

int main() {
    drogular::App app;

    app.staticFiles(
       "/assets",
       "examples/todo_pwa/public"
   )
   .staticFileCacheProfile(
       drogular::StaticFileCacheProfile::Development
   );

    app.services().add<TodoStore>(
        drogular::ServiceLifetime::Singleton
    );

    app.component<TodoItemComponent>();

    app.page<TodoPage>("/");
    app.action<CreateTodoAction>("/todos/create");
    app.action<ToggleTodoAction>("/todos/toggle");
    app.action<DeleteTodoAction>("/todos/delete");

    app.run(8080);
}