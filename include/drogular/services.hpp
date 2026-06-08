#pragma once

#include <memory>

namespace drogular {

class GraphQLClient;

/**
 * Stores application-wide services.
 *
 * This is the foundation for a future dependency injection container.
 */
class ApplicationServices {
public:
    /**
     * Sets the GraphQL client.
     */
    void setGraphQLClient(std::shared_ptr<GraphQLClient> client);

    /**
     * Returns the GraphQL client or nullptr if it is not configured.
     */
    GraphQLClient* graphQLClient() const;

private:
    std::shared_ptr<GraphQLClient> graphQLClient_;
};

} // namespace drogular
