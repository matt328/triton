#include "dd/DrawContextFactory.hpp"
#include "dd/DrawContext.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "dd/buffer-registry/BufferRegistry.hpp"
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

    const auto geometryBufferHandle = bufferRegistry->getOrCreateBuffer(
        GeometryBufferConfig{.vertexFormat = renderConfig.vertexFormat});

    // Don't want to reuse object buffer, each drawContext should always get its own.
    // Still need the config here to tell it how to create the buffer
    const auto logicalObjectBufferHandle = frameManager->createPerFrameBuffer(ObjectBufferConfig{
        .hasMaterialId = renderConfig.objectDataType == ObjectDataType::BaseMaterial ||
                         renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated,
        .hasAnimationDataId = renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated});

    const auto materialBufferHandle =
        bufferRegistry->getOrCreateBuffer(MaterialBufferConfig{.id = 1});

    const auto dcci = DrawContextCreateInfo{
        .geometryBufferHandle = geometryBufferHandle,
        .materialBufferHandle = materialBufferHandle,
        .objectDataBufferHandle = logicalObjectBufferHandle,
    };

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>(dcci));
  }

  return drawContexts.at(renderConfigHandle).get();
}

}
