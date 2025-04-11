#include "dd/DrawContextFactory.hpp"
#include "api/gfx/GpuObjectData.hpp"
#include "dd/DrawContext.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "dd/buffer-registry/BufferRegistry.hpp"
#include "dd/buffer-registry/GeometryRregionBufferConfig.hpp"
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
    const auto geometryBufferHandle = bufferRegistry->getOrCreateBuffer(
        GeometryBufferConfig{.vertexFormat = renderConfig.vertexFormat});

    // Shared across DrawContexts and Frames
    const auto materialBufferHandle =
        bufferRegistry->getOrCreateBuffer(MaterialBufferConfig{.id = 1});

    // Unique Per DrawContext, shared across Frames
    const auto geometryRegionBufferHandle =
        bufferRegistry->getOrCreateBuffer(GeometryRegionBufferConfig{.id = 1}, drawContextId);

    // One per DrawContext and one per frame
    // TODO(matt): finish other cases' strides
    const auto stride = sizeof(StaticGpuObjectData) * 1024;
    const auto logicalObjectBufferHandle = frameManager->createPerFrameBuffer(
        ObjectBufferConfig{.stride = stride,
                           .hasMaterialId =
                               renderConfig.objectDataType == ObjectDataType::BaseMaterial ||
                               renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated,
                           .hasAnimationDataId =
                               renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated},
        drawContextId);

    const auto dcci = DrawContextCreateInfo{
        .id = drawContextId,
        .geometryBufferHandle = geometryBufferHandle,
        .materialBufferHandle = materialBufferHandle,
        .geometryRegionBufferHandle = geometryRegionBufferHandle,
        .objectDataBufferHandle = logicalObjectBufferHandle,
    };

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>(dcci));
  }

  return drawContexts.at(renderConfigHandle).get();
}

}
