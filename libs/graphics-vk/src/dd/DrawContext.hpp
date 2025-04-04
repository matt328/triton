#pragma once

#include "dd/Renderable.hpp"
#include "mem/Buffer.hpp"

namespace tr {

class DrawContext {
public:
  DrawContext() = default;
  ~DrawContext() = default;

  DrawContext(const DrawContext&) = delete;
  DrawContext(DrawContext&&) = delete;
  auto operator=(const DrawContext&) -> DrawContext& = delete;
  auto operator=(DrawContext&&) -> DrawContext& = delete;

  auto writeObjectData(const Renderable& renderable) -> void;

private:
  std::unique_ptr<Buffer> objectDataBuffer;
};

}
