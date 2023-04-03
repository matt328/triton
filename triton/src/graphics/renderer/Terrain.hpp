#pragma once

#include "graphics/renderer/RendererBase.hpp"

class Terrain : public RendererBase {
 public:
   Terrain(const RendererBaseCreateInfo& createInfo);
   ~Terrain() override;

   Terrain(const Terrain&) = delete;
   Terrain(Terrain&&) = delete;
   Terrain& operator=(const Terrain&) = delete;
   Terrain& operator=(Terrain&&) = delete;

   void fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) override;
   void update() override;
};
