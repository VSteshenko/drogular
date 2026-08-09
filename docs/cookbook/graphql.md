# GraphQL

## Problem

**Need to communicate with a GraphQL server?**

This guide shows how to organize GraphQL operations without spreading request construction and JSON mapping throughout the application.

---

## Recommended Solution

Define reusable GraphQL documents, execute them through `GraphQLClient`, and isolate response mapping behind application providers.

This keeps pages and actions independent from GraphQL transport details while allowing the same application code to work with different client implementations.

---

## How It Works

### GraphQL Client

`GraphQLClient` defines the common interface for executing queries, mutations, and complete requests.

```cpp
class GraphQLClient {
public:
    virtual ~GraphQLClient() = default;

    virtual GraphQLResponse execute(
        const gql::Query& query,
        const GraphQLVariables& variables = {}
    ) = 0;

    virtual GraphQLResponse execute(
        const gql::Mutation& mutation,
        const GraphQLVariables& variables = {}
    ) = 0;

    virtual GraphQLResponse executeRequest(
        const GraphQLRequest& request
    ) = 0;
};
```

Application code depends on this interface rather than a concrete transport.

Drogular provides `HttpGraphQLClient` for HTTP communication and `InProcessGraphQLClient` for executing operations inside the same process.

---

### GraphQL Documents

Keep queries and mutations in dedicated document classes.

PortalDemo defines user queries through `UserQueries`:

```cpp
class UserQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query search(
        const PortalUserQuery& query
    );

    static drogular::gql::Query findByCredentials();
};
```

The document builder keeps operation names, variables, arguments, and selected fields together.

```cpp
return drogular::gql::query("PortalUserByCredentials")
    .variable("username", "String!")
    .variable("password", "String!")
    .select(
        drogular::gql::field("userByCredentials")
            .arg(
                "username",
                drogular::gql::variable("username")
            )
            .arg(
                "password",
                drogular::gql::variable("password")
            )
            .children(
                PortalGraphQLSelectionBuilder::from(
                    PortalSchema::users()
                )
            )
    );
```

Queries and mutations remain reusable and can be tested independently from the page or action that needs them.

---

### Variables

Pass operation values through `GraphQLVariables` instead of embedding them into the document text.

```cpp
const auto response =
    client_->execute(
        UserQueries::findByCredentials(),
        UserMapper::credentialsVariables(
            username,
            password
        )
    );
```

PortalDemo keeps variable construction inside mappers so that GraphQL-specific names and JSON values do not leak into the rest of the application.

---

### Response Mapping

`GraphQLResponse` exposes the GraphQL data, errors, and individual fields.

```cpp
const auto users =
    response.field("users");

if (!users.has_value()) {
    return {};
}

return UserMapper::fromList(*users);
```

Map response values into application models before returning them to pages or actions.

```text
GraphQLResponse
       │
       ▼
Mapper
       │
       ▼
Application Model
```

This keeps the rest of the application independent from the JSON structure returned by GraphQL.

---

## Example

PortalDemo implements `PortalGraphQLUserProvider` as a GraphQL-backed version of `PortalUserProvider`.

```cpp
class PortalGraphQLUserProvider final
    : public PortalUserProvider
{
public:
    explicit PortalGraphQLUserProvider(
        std::shared_ptr<drogular::GraphQLClient> client
    )
        : client_(std::move(client))
    {
    }

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};
```

The provider executes a reusable query, supplies variables, and maps the response into a paged application result.

```cpp
drogular::PagedResult<PortalUser> search(
    const PortalUserQuery& query
) const override {
    const auto response =
        client_->execute(
            UserQueries::search(query),
            UserMapper::toVariables(query)
        );

    const auto page =
        response.field("userPage");

    if (!page.has_value()) {
        return {};
    }

    return UserMapper::pageFromValue(*page);
}
```

The complete request flow remains explicit:

```text
Page or Action
      │
      ▼
Application Provider
      │
      ├── GraphQL Document
      ├── GraphQL Variables
      │
      ▼
GraphQLClient
      │
      ▼
GraphQLResponse
      │
      ▼
Mapper
      │
      ▼
Application Model
```

Pages and actions depend on the application provider. They do not construct GraphQL documents or inspect raw response JSON.

---

### Mutations

Use the same structure for operations that modify data.

```cpp
PortalUser create(
    const PortalUserCreate& input
) override {
    PortalUser user;

    user.username = input.username;
    user.password = input.password;
    user.role = input.role;

    const auto response =
        client_->execute(
            UserMutations::create(user),
            UserMapper::toVariables(user)
        );

    return UserMapper::fromValue(
        response.data()["createUser"]
    );
}
```

The mutation document, variables, execution, and result mapping stay inside the GraphQL-backed provider.

---

### Error Handling

`HttpGraphQLClient` throws `GraphQLClientError` when a response contains GraphQL errors.

Transport failures and invalid responses should be handled at the application boundary that invokes the provider.

Keep GraphQL error details out of templates and components. Convert them into application-specific results or validation messages before rendering a response.

---

## Best Practices

- Depend on `GraphQLClient`, not a concrete client implementation.
- Keep queries and mutations in dedicated document classes.
- Pass dynamic values through `GraphQLVariables`.
- Keep GraphQL field names and JSON conversion inside mappers.
- Return application models from GraphQL-backed providers.
- Keep pages and actions independent from raw GraphQL responses.
- Test documents, mappers, and providers separately.

---

## See Also

### Getting Started

- [Project Structure](../getting-started/project-structure.md)

### API Reference

- [GraphQL](../reference/graphql/README.md)
- [GraphQLClient](../reference/graphql/client/graphql-client.md)
- [HttpGraphQLClient](../reference/graphql/client/http-graphql-client.md)
- [InProcessGraphQLClient](../reference/graphql/client/in-process-graphql-client.md)
- [GraphQLRequest](../reference/graphql/requests/graphql-request.md)
- [GraphQLResponse](../reference/graphql/requests/graphql-response.md)
- [GraphQLVariables](../reference/graphql/requests/graphql-variables.md)
- [gql::Query](../reference/graphql/documents/query.md)
- [gql::Mutation](../reference/graphql/documents/mutation.md)
