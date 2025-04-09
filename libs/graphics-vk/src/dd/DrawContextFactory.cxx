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

    const auto geometryBufferConfig =
        GeometryBufferConfig{.vertexFormat = renderConfig.vertexFormat};

    const auto geometryBufferHandle = bufferRegistry->getOrCreateBuffer(geometryBufferConfig);

    const auto objectBufferConfig = ObjectBufferConfig{
        .hasMaterialId = renderConfig.objectDataType == ObjectDataType::BaseMaterial ||
                         renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated,
        .hasAnimationDataId = renderConfig.objectDataType == ObjectDataType::BaseMaterialAnimated};

    const auto logicalObjectBufferHandle =
        frameManager->getOrCreatePerFrameBuffer(objectBufferConfig);

    const auto materialBufferHandle = bufferRegistry->getOrCreateMaterialBuffer();

    const auto dcci = DrawContextCreateInfo{.geometryBufferHandle = geometryBufferHandle,
                                            .objectDataBufferHandle = logicalObjectBufferHandle,
                                            .materialBufferHandle = materialBufferHandle};

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>(dcci));
  }

  return drawContexts.at(renderConfigHandle).get();
}

}
