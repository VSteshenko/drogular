#pragma once

#include "todo.hpp"

#include <drogular/state.hpp>

#include <algorithm>
#include <string>
#include <vector>

class TodoStore {
public:
    TodoStore()
        : TodoStore(
              std::vector<Todo>{
                  {1, "Create Drogular project skeleton", true},
                  {2, "Build TodoPWA example", false},
                  {3, "Add actions and mutations", false},
                  {4, "Experiment with State Management", false}
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

    drogular::State<std::vector<Todo>> todos;

private:
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