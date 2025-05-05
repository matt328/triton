#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "r3/draw-context/DrawContext.hpp"

namespace tr {

class DrawContext;

class DrawContextFactory {
public:
  explicit DrawContextFactory(std::shared_ptr<BufferSystem> newBufferSystem);
  ~DrawContextFactory() = default;

  DrawContextFactory(const DrawContextFactory&) = delete;
  DrawContextFactory(DrawContextFactory&&) = delete;
  auto operator=(const DrawContextFactory&) -> DrawContextFactory& = delete;
  auto operator=(DrawContextFactory&&) -> DrawContextFactory& = delete;

  auto createDrawContext(DrawContextConfig& config) -> Handle<DrawContext>;
  auto getDrawContext(const Handle<DrawContext>& handle) -> std::unique_ptr<DrawContext>&;

private:
  std::shared_ptr<BufferSystem> bufferSystem;

  HandleGenerator<DrawContext> drawHandleGenerator;
  std::unordered_map<Handle<DrawContext>, std::unique_ptr<DrawContext>> drawContextMap;
};

}
