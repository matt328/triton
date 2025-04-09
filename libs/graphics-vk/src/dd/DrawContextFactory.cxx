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

    /*
      FrameManager will have to have a method to getOrCreatePerFrameBuffer(objectBufferConfig);
      this method will return a LogicalBufferHandle.

      during rendering, DrawContext can just do h = frame->getBufferHandle(logicalHandle)
      then bufferregistry->getBuffer(h);

      Should this use bufferregistry? I think so? BufferRegistry is the higher level abstraction
      over the low level BufferManager? Need to think about the 'levels' of buffer-ness here. I
      think it should be flattened so that there is one 'thing' that owns all the buffers, whether
      they're arena, arenageometry, or regular.
    */

    const auto materialBufferHandle = bufferRegistry->getOrCreateMaterialBuffer();

    const auto dcci = DrawContextCreateInfo{.geometryBufferHandle = geometryBufferHandle,
                                            .objectDataBufferHandle = objectBufferHandle,
                                            .materialBufferHandle = materialBufferHandle};

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>(dcci));
  }

  return drawContexts.at(renderConfigHandle).get();
}

}
