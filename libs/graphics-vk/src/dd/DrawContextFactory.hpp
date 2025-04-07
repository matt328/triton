#pragma once

#include "dd/RenderConfigHandle.hpp"

namespace tr {

using DrawContextHandle = size_t;

class DrawContext;
class RenderConfigRegistry;

class DrawContextFactory {
public:
  explicit DrawContextFactory(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry);
  ~DrawContextFactory() = default;

  DrawContextFactory(const DrawContextFactory&) = delete;
  DrawContextFactory(DrawContextFactory&&) = delete;
  auto operator=(const DrawContextFactory&) -> DrawContextFactory& = delete;
  auto operator=(DrawContextFactory&&) -> DrawContextFactory& = delete;

  auto getOrCreateDrawContext(RenderConfigHandle renderConfigHandle) -> DrawContext*;

private:
  std::shared_ptr<RenderConfigRegistry> renderConfigRegistry;

  std::unordered_map<RenderConfigHandle, std::unique_ptr<DrawContext>> drawContexts;
};

}
