#include <drogular/dependency_graph.hpp>

#include <gtest/gtest.h>

class GraphService {};
class GraphRepository {};
class GraphLogger {};

TEST(CoreDependencyGraphTests, StoresDependency) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphService, GraphRepository>();

    EXPECT_TRUE(
        graph.dependsOn(
            std::type_index(typeid(GraphService)),
            std::type_index(typeid(GraphRepository))
        )
    );
}

TEST(CoreDependencyGraphTests, ReturnsDependencies) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphService, GraphRepository>();
    graph.addDependency<GraphService, GraphLogger>();

    const auto dependencies =
        graph.dependencies<GraphService>();

    EXPECT_EQ(dependencies.size(), 2);
}

class GraphA {};
class GraphB {};
class GraphC {};

TEST(CoreDependencyGraphTests, DetectsCircularDependency) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphA, GraphB>();
    graph.addDependency<GraphB, GraphC>();
    graph.addDependency<GraphC, GraphA>();

    EXPECT_TRUE(graph.hasCircularDependencies());
}

TEST(CoreDependencyGraphTests, ReturnsFalseWhenGraphHasNoCycle) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphA, GraphB>();
    graph.addDependency<GraphB, GraphC>();

    EXPECT_FALSE(graph.hasCircularDependencies());
}

TEST(CoreDependencyGraphTests, ReturnsCircularDependencyPath) {
    drogular::DependencyGraph graph;

    graph.addDependency<GraphA, GraphB>();
    graph.addDependency<GraphB, GraphC>();
    graph.addDependency<GraphC, GraphA>();

    const auto path = graph.circularDependencyPath();

    EXPECT_FALSE(path.empty());
}