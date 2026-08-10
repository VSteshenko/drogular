#pragma once

#include "todo.hpp"
#include "todo_query.hpp"

#include <drogular/pagination.hpp>
#include <drogular/state.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

class TodoStore {
public:
    using Callback = std::function<void(const std::vector<Todo>&)>;

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
        : todos_(std::move(initialTodos)) {
        updateNextId();
    }

    TodoStore(const TodoStore&) = delete;
    TodoStore& operator=(const TodoStore&) = delete;

    TodoStore(TodoStore&&) = delete;
    TodoStore& operator=(TodoStore&&) = delete;

    void create(std::string title) {
        if (title.empty()) {
            return;
        }

        std::vector<Todo> publishedTodos;
        std::vector<Callback> subscribers;

        {
            std::lock_guard lock(mutex_);
            auto updatedTodos = todos_.value();

            updatedTodos.push_back({
                nextId_++,
                std::move(title),
                false
            });

            todos_.set(std::move(updatedTodos));
            publishedTodos = todos_.value();
            subscribers = subscribers_;
        }

        notify(subscribers, publishedTodos);
    }

    void toggle(int id) {
        std::vector<Todo> publishedTodos;
        std::vector<Callback> subscribers;

        {
            std::lock_guard lock(mutex_);
            auto updatedTodos = todos_.value();

            for (auto& todo : updatedTodos) {
                if (todo.id == id) {
                    todo.done = !todo.done;
                    break;
                }
            }

            todos_.set(std::move(updatedTodos));
            publishedTodos = todos_.value();
            subscribers = subscribers_;
        }

        notify(subscribers, publishedTodos);
    }

    void remove(int id) {
        std::vector<Todo> publishedTodos;
        std::vector<Callback> subscribers;

        {
            std::lock_guard lock(mutex_);
            auto updatedTodos = todos_.value();

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

            todos_.set(std::move(updatedTodos));
            publishedTodos = todos_.value();
            subscribers = subscribers_;
        }

        notify(subscribers, publishedTodos);
    }

    std::vector<Todo> snapshot() const {
        std::lock_guard lock(mutex_);
        return todos_.value();
    }

    void subscribe(Callback callback) {
        std::lock_guard lock(mutex_);
        subscribers_.push_back(std::move(callback));
    }

    drogular::PagedResult<Todo> find(const TodoQuery& query) const {
        std::lock_guard lock(mutex_);

        std::vector<Todo> filtered;
        filtered.reserve(todos_.value().size());

        const auto search = lowercase(query.search);

        for (const auto& todo : todos_.value()) {
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


    static void notify(
        const std::vector<Callback>& subscribers,
        const std::vector<Todo>& todos
    ) {
        for (const auto& subscriber : subscribers) {
            subscriber(todos);
        }
    }

    void updateNextId() {
        nextId_ = 1;

        for (const auto& todo : todos_.value()) {
            if (todo.id >= nextId_) {
                nextId_ = todo.id + 1;
            }
        }
    }

    mutable std::mutex mutex_;
    drogular::State<std::vector<Todo>> todos_;
    std::vector<Callback> subscribers_;
    int nextId_ = 1;
};