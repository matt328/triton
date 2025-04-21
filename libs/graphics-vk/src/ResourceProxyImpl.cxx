#include "ResourceProxyImpl.hpp"

#include "VkResourceManager.hpp"
#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"
#include "geo/DynamicGeometryData.hpp"
#include "geo/StaticGeometryData.hpp"
#include "gfx/IRenderContext.hpp"

namespace tr {

/// This ProxyImpl keeps vk-graphics from knowing about as::Model
/// Is only called from game-world
ResourceProxyImpl::ResourceProxyImpl(std::shared_ptr<VkResourceManager> newResourceManager,
                                     std::shared_ptr<IRenderContext> newRenderContext)
    : resourceManager{std::move(newResourceManager)}, renderContext{std::move(newRenderContext)} {
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

auto ResourceProxyImpl::uploadGeometry(DDGeometryData&& data) -> MeshHandle {

  /*
  This method needs to figure out which buffers to upload data to.
  It should look at the vertex list, and determine what data needs to go into what buffers
  */

  for (const auto& attribute : data.getVertexList().format.attributes) {
    const auto x = data.getVertexList().getData(attribute);
  }

  const auto renderableData = RenderableData{
      .geometryData = std::move(data),
      .objectData = ObjectData{.objectDataBytes = {}},
      .materialData = MaterialData{.shadingMode = ShadingMode::Wireframe},
  };
  [[maybe_unused]] const auto handles = renderContext->registerRenderable(renderableData);
  // Log.debug("handles: {}", handles);
  return resourceManager->uploadGeometryData(data);
}

}
