#include "RenderPassFactory.hpp"
#include "r3/render-pass/GraphicsPassConfig.hpp"

namespace tr {

auto RenderPassFactory::createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
    -> std::unique_ptr<GraphicsPass> {
  Log.trace("RenderPassFactory::createGraphicsPass()");

  const auto config = GraphicsPassConfig{.id = 4};

  return std::make_unique<GraphicsPass>(config);
}

}
