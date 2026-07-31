#include <drogular/in_process_graphql_client.hpp>

#include <cctype>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace drogular {
namespace {

enum class OperationKind {
    Query,
    Mutation
};

struct ParsedOperation {
    OperationKind kind;
    std::string name;
};

class OperationHeaderParser {
public:
    explicit OperationHeaderParser(std::string_view document)
        : document_(document) {
    }

    ParsedOperation parse() {
        skipIgnored();

        if (peek() == '{') {
            throw GraphQLClientError(
                "InProcessGraphQLClient requires a named GraphQL operation"
            );
        }

        const auto operationType = readName();
        OperationKind kind;

        if (operationType == "query") {
            kind = OperationKind::Query;
        } else if (operationType == "mutation") {
            kind = OperationKind::Mutation;
        } else {
            throw GraphQLClientError(
                "Unsupported GraphQL operation type: " + operationType
            );
        }

        skipIgnored();
        const auto operationName = readName();
        if (operationName.empty()) {
            throw GraphQLClientError(
                "InProcessGraphQLClient requires a named GraphQL operation"
            );
        }

        skipIgnored();
        const auto next = peek();
        if (next != '(' && next != '{' && next != '@') {
            throw GraphQLClientError(
                "Invalid GraphQL operation header"
            );
        }

        return {
            .kind = kind,
            .name = operationName
        };
    }

private:
    char peek() const {
        return position_ < document_.size()
            ? document_[position_]
            : '\0';
    }

    void skipIgnored() {
        while (position_ < document_.size()) {
            const auto current = document_[position_];

            if (std::isspace(static_cast<unsigned char>(current)) ||
                current == ',') {
                ++position_;
                continue;
            }

            if (current == '#') {
                while (position_ < document_.size() &&
                       document_[position_] != '\n' &&
                       document_[position_] != '\r') {
                    ++position_;
                }
                continue;
            }

            break;
        }
    }

    std::string readName() {
        if (position_ >= document_.size()) {
            return {};
        }

        const auto first = document_[position_];
        if (!(std::isalpha(static_cast<unsigned char>(first)) ||
              first == '_')) {
            return {};
        }

        const auto start = position_++;
        while (position_ < document_.size()) {
            const auto current = document_[position_];
            if (!(std::isalnum(static_cast<unsigned char>(current)) ||
                  current == '_')) {
                break;
            }
            ++position_;
        }

        return std::string(document_.substr(start, position_ - start));
    }

    std::string_view document_;
    std::size_t position_{0};
};

GraphQLVariables toVariables(const Json::Value& values) {
    GraphQLVariables variables;

    if (!values.isObject()) {
        return variables;
    }

    for (const auto& name : values.getMemberNames()) {
        variables.set(name, values[name]);
    }

    return variables;
}

} // namespace

InProcessGraphQLClient::InProcessGraphQLClient(
    std::shared_ptr<GraphQLServer> server,
    ContextFactory contextFactory
)
    : server_(std::move(server)),
      contextFactory_(std::move(contextFactory)) {
    if (!server_) {
        throw std::invalid_argument(
            "InProcessGraphQLClient requires a non-null GraphQLServer"
        );
    }
}

GraphQLResponse InProcessGraphQLClient::execute(
    const gql::Query& query,
    const GraphQLVariables& variables
) {
    return server_->executeQuery(
        query.name(),
        variables,
        createContext()
    );
}

GraphQLResponse InProcessGraphQLClient::execute(
    const gql::Mutation& mutation,
    const GraphQLVariables& variables
) {
    return server_->executeMutation(
        mutation.name(),
        variables,
        createContext()
    );
}

GraphQLResponse InProcessGraphQLClient::executeRequest(
    const GraphQLRequest& request
) {
    const auto operation =
        OperationHeaderParser(request.query()).parse();
    const auto variables = toVariables(request.variables());
    const auto context = createContext();

    if (operation.kind == OperationKind::Mutation) {
        return server_->executeMutation(
            operation.name,
            variables,
            context
        );
    }

    return server_->executeQuery(
        operation.name,
        variables,
        context
    );
}

GraphQLExecutionContext
InProcessGraphQLClient::createContext() const {
    return contextFactory_
        ? contextFactory_()
        : GraphQLExecutionContext{};
}

} // namespace drogular