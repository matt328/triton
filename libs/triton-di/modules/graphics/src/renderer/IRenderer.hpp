#pragma once

namespace tr::gfx::sb {
   class ShaderBinding;
}

namespace tr::cm::gpu {
   class MeshData;
}

namespace tr::gfx::rd {
   class IRenderer {
    public:
      IRenderer() = default;
      virtual ~IRenderer() = default;

      IRenderer(const IRenderer&) = default;
      IRenderer(IRenderer&&) = delete;
      auto operator=(const IRenderer&) -> IRenderer& = default;
      auto operator=(IRenderer&&) -> IRenderer& = delete;

      virtual void bindPipeline(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) = 0;
      virtual void applyShaderBinding(
          const std::unique_ptr<sb::ShaderBinding>& binding,
          uint32_t setIndex,
          const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) = 0;
      virtual void render(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer,
                          std::span<cm::gpu::MeshData> meshDataList,
                          const std::tuple<vk::Viewport, vk::Rect2D>& vpScissor) = 0;
   };
}
