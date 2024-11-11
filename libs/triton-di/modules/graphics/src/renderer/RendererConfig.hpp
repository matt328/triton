#pragma once

#include "geo/VertexAttributes.hpp"

namespace tr::gfx::rd {
   enum class RendererType : uint8_t {
      StaticModel = 1,
      AnimatedModel = 2,
      Terrain = 3,
      Line = 4,
      Debug = 5
   };

   struct RendererConfig {
      RendererType rendererType;
      std::vector<vk::DescriptorSetLayout> setLayouts;
      std::vector<geo::VertexComponent> vertexComponents;
      std::vector<vk::PushConstantRange> pushConstantRanges;
   };
}