#include "todo_service.hpp"

#include <utility>
#include <algorithm>

TodoService::TodoService()
    : todos_{
          {1, "Create Drogular project skeleton", true},
          {2, "Build TodoPWA example", false},
          {3, "Add actions and mutations", false}
      },
      nextId_(4) {
}

TodoService::TodoService(std::vector<Todo> todos)
    : todos_(std::move(todos)) {
    for (const auto& todo : todos_) {
        if (todo.id >= nextId_) {
            nextId_ = todo.id + 1;
        }
    }
}

const std::vector<Todo>& TodoService::todos() const {
    return todos_;
}

void TodoService::create(std::string title) {
    if (title.empty()) {
        return;
    }

    todos_.push_back({
        nextId_++,
        std::move(title),
        false
    });
}
