#pragma once

#include "RendererConfig.hpp"

namespace tr {
class IGraphicsDevice;
}

namespace tr {
class IShaderCompiler;
}

namespace tr {

class IRenderer;

class RendererFactory {
public:
  RendererFactory(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                  std::shared_ptr<IShaderCompiler> newShaderCompiler);
  ~RendererFactory() = default;

  RendererFactory(const RendererFactory&) = default;
  RendererFactory(RendererFactory&&) = delete;
  auto operator=(const RendererFactory&) -> RendererFactory& = default;
  auto operator=(RendererFactory&&) -> RendererFactory& = delete;

  auto createRenderer(RendererConfig config) -> std::shared_ptr<IRenderer>;

private:
  std::shared_ptr<IGraphicsDevice> graphicsDevice;
  std::shared_ptr<IShaderCompiler> shaderCompiler;
};
}
