#pragma once

#include "todo.hpp"

#include <string>
#include <vector>

class TodoService {
public:
    TodoService();

    explicit TodoService(std::vector<Todo> todos);

    const std::vector<Todo>& todos() const;

    void create(std::string title);

private:
    std::vector<Todo> todos_;
    int nextId_ = 1;
};