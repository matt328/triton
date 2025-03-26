#include "ResourceProxyImpl.hpp"

#include "VkResourceManager.hpp"
#include "geo/DynamicGeometryData.hpp"
#include "geo/StaticGeometryData.hpp"

namespace tr {

/// This ProxyImpl keeps vk-graphics from knowing about as::Model
/// Is only called from game-world
ResourceProxyImpl::ResourceProxyImpl(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)} {
}

auto ResourceProxyImpl::uploadModel(const as::Model& model) -> ModelData {
  MeshHandle meshHandle = 0L;

  if (model.skinned()) {
    auto verts = model.dynamicVertices.value();
    auto indices = model.indices;
    auto geometryData = DynamicGeometryData{std::move(verts), std::move(indices)};
    meshHandle = resourceManager->uploadDynamicMesh(geometryData);
  } else {
    auto verts = model.staticVertices.value();
    auto indices = model.indices;
    auto geometryData = StaticGeometryData{std::move(verts), std::move(indices)};
    meshHandle = resourceManager->uploadStaticMesh(geometryData);
  }

  auto textureHandle = resourceManager->uploadImage(model.imageData, "unnamed_texture");
  auto meshData = MeshData{.meshHandle = meshHandle,
                           .topology = Topology::Triangles,
                           .textureHandle = textureHandle};

  auto skinData = SkinData{
      .jointMap = model.jointRemaps,
      .inverseBindMatrices = model.inverseBindPoses,
  };
  return ModelData{.meshData = meshData, .skinData = skinData, .animationData = std::nullopt};
}

}
