#pragma once

#include "gfx/dss/DescriptorSetLayoutCache.hpp"

namespace tr::gfx {
   class GraphicsDevice;
   class Pipeline;
}

namespace tr::gfx::dss {
   class DescriptorSetLayoutCache;
   class DescriptorAllocator;
}

namespace tr::gfx::render {
   class StaticMeshRenderer {
    public:
      StaticMeshRenderer(const GraphicsDevice& graphicsDevice,
                         dss::DescriptorSetLayoutCache* layoutCache);
      ~StaticMeshRenderer();

      StaticMeshRenderer(const StaticMeshRenderer&) = delete;
      StaticMeshRenderer& operator=(const StaticMeshRenderer&) = delete;

      StaticMeshRenderer(StaticMeshRenderer&&) = delete;
      StaticMeshRenderer& operator=(StaticMeshRenderer&&) = delete;

      void draw(const vk::raii::CommandBuffer& cmd);

      void resize(const vk::Extent2D newSize);

    private:
      const GraphicsDevice& graphicsDevice;
      dss::DescriptorSetLayoutCache* layoutCache;
      std::unique_ptr<Pipeline> pipeline;

      void initPipeline(const GraphicsDevice& graphicsDevice,
                        dss::DescriptorSetLayoutCache* layoutCache);
   };
}