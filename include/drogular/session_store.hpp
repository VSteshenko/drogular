#pragma once

#include <drogular/session.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace drogular {

class SessionStore {
public:
    std::shared_ptr<Session> create();

    std::shared_ptr<Session> get(
        const std::string& id
    );

    bool contains(
        const std::string& id
    ) const;

    void remove(
        const std::string& id
    );

    void clear();

private:
    std::string generateId() const;

    std::unordered_map<
        std::string,
        std::shared_ptr<Session>
    > sessions_;
};

} // namespace drogular