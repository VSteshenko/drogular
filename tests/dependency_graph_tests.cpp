#include <drogular/dependency_graph.hpp>

#include <gtest/gtest.h>

class GraphService {};
class GraphRepository {};
class GraphLogger {};

TEST(DependencyGraphTests, StoresDependency) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphService, GraphRepository>();

    EXPECT_TRUE(
        graph.dependsOn(
            std::type_index(typeid(GraphService)),
            std::type_index(typeid(GraphRepository))
        )
    );
}

TEST(DependencyGraphTests, ReturnsDependencies) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphService, GraphRepository>();
    graph.addDependency<GraphService, GraphLogger>();

    const auto dependencies =
        graph.dependencies<GraphService>();

    EXPECT_EQ(dependencies.size(), 2);
}
