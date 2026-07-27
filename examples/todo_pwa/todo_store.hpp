#pragma once

#include "todo.hpp"
#include "todo_query.hpp"

#include <drogular/pagination.hpp>
#include <drogular/state.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

class TodoStore {
public:
    TodoStore()
        : TodoStore(
              std::vector<Todo>{
                  {1, "Create Drogular project skeleton", true},
                  {2, "Build TodoPWA example", true},
                  {3, "Add actions and mutations", true},
                  {4, "Experiment with State Management", true},
                  {5, "Add form validation", true},
                  {6, "Add offline support", true},
                  {7, "Write TodoPWA tests", true},
                  {8, "Improve component lifecycle", false},
                  {9, "Document dependency injection", false},
                  {10, "Add localization example", false},
                  {11, "Create reusable pagination", false},
                  {12, "Add todo search", false},
                  {13, "Support query serialization", false},
                  {14, "Preserve filters between pages", false},
                  {15, "Add empty search state", false},
                  {16, "Test invalid page parameters", false},
                  {17, "Refine TodoPWA styling", false},
                  {18, "Review public API naming", false}
              }
          ) {
    }

    explicit TodoStore(std::vector<Todo> initialTodos)
        : todos(std::move(initialTodos)) {
        updateNextId();
    }

    TodoStore(const TodoStore&) = delete;
    TodoStore& operator=(const TodoStore&) = delete;

    TodoStore(TodoStore&&) = default;
    TodoStore& operator=(TodoStore&&) = default;

    void create(std::string title) {
        if (title.empty()) {
            return;
        }

        auto updatedTodos = todos.value();

        updatedTodos.push_back({
            nextId_++,
            std::move(title),
            false
        });

        todos.set(std::move(updatedTodos));
    }

    void toggle(int id) {
        auto updatedTodos = todos.value();

        for (auto& todo : updatedTodos) {
            if (todo.id == id) {
                todo.done = !todo.done;
                break;
            }
        }

        todos.set(std::move(updatedTodos));
    }

    void remove(int id) {
        auto updatedTodos = todos.value();

        updatedTodos.erase(
            std::remove_if(
                updatedTodos.begin(),
                updatedTodos.end(),
                [id](const Todo& todo) {
                    return todo.id == id;
                }
            ),
            updatedTodos.end()
        );

        todos.set(std::move(updatedTodos));
    }

    drogular::PagedResult<Todo> find(const TodoQuery& query) const {
        std::vector<Todo> filtered;
        filtered.reserve(todos.value().size());

        const auto search = lowercase(query.search);

        for (const auto& todo : todos.value()) {
            if (search.empty() ||
                lowercase(todo.title).find(search) != std::string::npos) {
                filtered.push_back(todo);
            }
        }

        return drogular::paginate(
            filtered,
            query.page,
            query.pageSize
        );
    }

    drogular::State<std::vector<Todo>> todos;

private:
    static std::string lowercase(std::string value) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );
        return value;
    }

    void updateNextId() {
        nextId_ = 1;

        for (const auto& todo : todos.value()) {
            if (todo.id >= nextId_) {
                nextId_ = todo.id + 1;
            }
        }
    }

    int nextId_ = 1;
};