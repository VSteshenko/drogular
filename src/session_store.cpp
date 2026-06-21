#include <drogular/session_store.hpp>

#include <random>
#include <sstream>

namespace drogular {

std::shared_ptr<Session> SessionStore::create() {
    auto id = generateId();

    while (sessions_.find(id) != sessions_.end()) {
        id = generateId();
    }

    auto session =
        std::make_shared<Session>();

    session->set("_id", id);

    sessions_[id] = session;

    return session;
}

std::shared_ptr<Session> SessionStore::get(
    const std::string& id
) {
    const auto found =
        sessions_.find(id);

    if (found == sessions_.end()) {
        return nullptr;
    }

    return found->second;
}

bool SessionStore::contains(
    const std::string& id
) const {
    return sessions_.find(id) != sessions_.end();
}

void SessionStore::remove(
    const std::string& id
) {
    sessions_.erase(id);
}

void SessionStore::clear() {
    sessions_.clear();
}

std::string SessionStore::generateId() const {
    static thread_local std::mt19937_64 generator{
        std::random_device{}()
    };

    std::uniform_int_distribution<
        unsigned long long
    > distribution;

    std::ostringstream stream;
    stream << std::hex << distribution(generator);

    return stream.str();
}

} // namespace drogular