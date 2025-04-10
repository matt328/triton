#pragma once

#include "bk/Rando.hpp"
#include "dd/RenderConfigHandle.hpp"

namespace tr {

using DrawContextHandle = size_t;

class DrawContext;
class RenderConfigRegistry;
class BufferRegistry;
class IFrameManager;

class DrawContextFactory {
public:
  DrawContextFactory(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                     std::shared_ptr<BufferRegistry> newBufferRegistry,
                     std::shared_ptr<IFrameManager> newFrameManager);
  ~DrawContextFactory() = default;

  DrawContextFactory(const DrawContextFactory&) = delete;
  DrawContextFactory(DrawContextFactory&&) = delete;
  auto operator=(const DrawContextFactory&) -> DrawContextFactory& = delete;
  auto operator=(DrawContextFactory&&) -> DrawContextFactory& = delete;

  auto getOrCreateDrawContext(RenderConfigHandle renderConfigHandle) -> DrawContext*;

private:
  std::shared_ptr<RenderConfigRegistry> renderConfigRegistry;
  std::shared_ptr<BufferRegistry> bufferRegistry;
  std::shared_ptr<IFrameManager> frameManager;

  MapKey drawContextKeygen;

  std::unordered_map<RenderConfigHandle, std::unique_ptr<DrawContext>> drawContexts;
};

}
