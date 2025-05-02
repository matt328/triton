#pragma once

#include "bk/Handle.hpp"
#include "r3/draw-context/DrawContext.hpp"

/*
  Considering whether to create the buffers up front in the renderer, have the creation of the
  DrawContexts create buffers as a side effect. I think the global buffers should be owned by the
  Renderer since it will need to update them, the DrawContexts just read their addresses to pass off
  to shaders via PushConstants.
*/

namespace tr {

class DrawContext;

class DrawContextFactory {
public:
  DrawContextFactory() = default;
  ~DrawContextFactory() = default;

  DrawContextFactory(const DrawContextFactory&) = default;
  DrawContextFactory(DrawContextFactory&&) = delete;
  auto operator=(const DrawContextFactory&) -> DrawContextFactory& = default;
  auto operator=(DrawContextFactory&&) -> DrawContextFactory& = delete;

  auto createDrawContext(DrawContextConfig& config) -> Handle<DrawContext>;
  auto getDrawContext(const Handle<DrawContext>& handle) -> DrawContext&;
};

}
