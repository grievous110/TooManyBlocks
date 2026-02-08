#include "StaticMeshBuilder.h"

#include "engine/blueprints/ChunkMeshBlueprint.h"
#include "engine/blueprints/StaticMeshBlueprint.h"

Future<StaticMesh::Internal> build(const Future<CPURenderData<Vertex>>& cpuMesh) {
    Future<std::shared_ptr<StaticMesh::Shared>> meshSharedData(
        [cpuMesh]() { return createSharedState(cpuMesh.value()); }, DEFAULT_TASKCONTEXT, Executor::Main
    );
    meshSharedData.dependsOn(cpuMesh);
    meshSharedData.start();

    Future<StaticMesh::Instance> meshInstanceData([cpuMesh]() { return createInstanceState(cpuMesh.value()); });
    meshInstanceData.dependsOn(cpuMesh);
    meshInstanceData.start();

    Future<StaticMesh::Internal> internal([meshSharedData, meshInstanceData] {
        return StaticMesh::Internal{meshSharedData.value(), meshInstanceData.value()};
    });
    internal.dependsOn(meshInstanceData).dependsOn(meshSharedData);

    return internal.start();
}

Future<StaticMesh::Internal> build(const Future<CPURenderData<CompactChunkVertex>>& cpuMesh) {
    Future<std::shared_ptr<StaticMesh::Shared>> meshSharedData(
        [cpuMesh]() { return createSharedState(cpuMesh.value()); }, DEFAULT_TASKCONTEXT, Executor::Main
    );
    meshSharedData.dependsOn(cpuMesh);
    meshSharedData.start();

    Future<StaticMesh::Instance> meshInstanceData([cpuMesh]() { return createInstanceState(cpuMesh.value()); });
    meshInstanceData.dependsOn(cpuMesh);
    meshInstanceData.start();

    Future<StaticMesh::Internal> internal([meshSharedData, meshInstanceData]() {
        return StaticMesh::Internal{meshSharedData.value(), meshInstanceData.value()};
    });
    internal.dependsOn(meshInstanceData).dependsOn(meshSharedData);
    internal.start();

    return internal;
}
