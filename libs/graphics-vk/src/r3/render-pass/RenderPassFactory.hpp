#pragma once

#include "r3/render-pass/GraphicsPassCreateInfo.hpp"

namespace tr {

class GraphicsPass;

class RenderPassFactory {
public:
  RenderPassFactory() = default;
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = default;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = default;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
      -> std::unique_ptr<GraphicsPass>;
};

}
