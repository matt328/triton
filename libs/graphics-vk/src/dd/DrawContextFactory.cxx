#include "dd/DrawContextFactory.hpp"
#include "api/gfx/GpuObjectData.hpp"
#include "dd/DrawContext.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "dd/buffer-registry/BufferRegistry.hpp"
#include "dd/buffer-registry/GeometryRregionBufferConfig.hpp"
#include "dd/gpu-data/GpuMaterialData.hpp"
#include "gfx/IFrameManager.hpp"

namespace tr {

DrawContextFactory::DrawContextFactory(
    std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
    std::shared_ptr<BufferRegistry> newBufferRegistry,
    std::shared_ptr<IFrameManager> newFrameManager)
    : renderConfigRegistry{std::move(newRenderConfigRegistry)},
      bufferRegistry{std::move(newBufferRegistry)},
      frameManager{std::move(newFrameManager)} {
}

auto DrawContextFactory::getOrCreateDrawContext(RenderConfigHandle renderConfigHandle)
    -> DrawContext* {

  if (!drawContexts.contains(renderConfigHandle)) {
    const auto renderConfig = renderConfigRegistry->get(renderConfigHandle);

    const auto drawContextId = drawContextKeygen.getKey();

    // Might be shared across multiple DrawContexts, Shared across Frames
    const auto geometryBufferHandle =
        bufferRegistry->getOrCreateBuffer(BufferConfig{.vertexFormat = renderConfig.vertexFormat});

    // Shared across DrawContexts and Frames
    const auto materialBufferHandle =
        bufferRegistry->getOrCreateBuffer(BufferConfig{.id = 1,
                                                       .size = sizeof(GpuMaterialData) * 10,
                                                       .bufferType = BufferType::Fixed});

    // Unique Per DrawContext, shared across Frames
    const auto geometryRegionBufferHandle =
        bufferRegistry->getOrCreateBuffer(GeometryRegionBufferConfig{.id = 1}, drawContextId);

    // One per DrawContext and one per frame
    const auto logicalObjectBufferHandle =
        frameManager->createPerFrameBuffer(getObjectBufferConfig(renderConfig), drawContextId);

    const auto logicalObjectIndexBufferHandle = frameManager->createPerFrameBuffer(
        StorageBufferConfig{.id = 2, .size = sizeof(uint32_t) * 1024},
        drawContextId);

    const auto logicalObjectCountBufferHandle =
        frameManager->createPerFrameBuffer(StorageBufferConfig{.id = 3, .size = sizeof(uint32_t)},
                                           drawContextId);

    const auto logicalIndirectoDrawCommandBufferHandle = frameManager->createPerFrameBuffer()

                                                             const auto dcci =
        DrawContextCreateInfo{.id = drawContextId,
                              .geometryBufferHandle = geometryBufferHandle,
                              .materialBufferHandle = materialBufferHandle,
                              .geometryRegionBufferHandle = geometryRegionBufferHandle,
                              .objectDataBufferHandle = logicalObjectBufferHandle,
                              .objectDataIndexBufferHandle = logicalObjectIndexBufferHandle,
                              .objectCountBufferHandle = logicalObjectCountBufferHandle};

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>(dcci));
  }

  return drawContexts.at(renderConfigHandle).get();
}

auto DrawContextFactory::getObjectBufferConfig(const RenderConfig& renderConfig)
    -> ObjectBufferConfig {
  // TODO(matt): finish other cases' strides
  const auto stride = sizeof(StaticGpuObjectData) * 1024;
  return {.stride = stride,
          .hasMaterialId = renderConfig.objectDataType == ObjectDataType::BaseMaterial ||
                           renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated,
          .hasAnimationDataId =
              renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated};
}
}
