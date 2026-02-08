#include "SkeletalMeshBuilder.h"

#include "engine/blueprints/SkeletalMeshBlueprint.h"

Future<SkeletalMesh::Internal> build(const Future<CPUSkeletalMeshData>& cpuMesh) {
    Future<std::shared_ptr<SkeletalMesh::Shared>> sharedMeshData(
        [cpuMesh]() { return createSharedState(cpuMesh.value()); }, DEFAULT_TASKCONTEXT, Executor::Main
    );
    sharedMeshData.dependsOn(cpuMesh);
    sharedMeshData.start();

    Future<SkeletalMesh::Instance> instanceMeshData(
        [cpuMesh]() { return createInstanceState(cpuMesh.value()); }, DEFAULT_TASKCONTEXT, Executor::Main
    );
    instanceMeshData.dependsOn(cpuMesh);
    instanceMeshData.start();

    Future<SkeletalMesh::Internal> internal([sharedMeshData, instanceMeshData]() mutable {
        return SkeletalMesh::Internal{sharedMeshData.value(), std::move(instanceMeshData.value())};
    });
    internal.dependsOn(sharedMeshData).dependsOn(instanceMeshData);
    
    return internal.start();
}