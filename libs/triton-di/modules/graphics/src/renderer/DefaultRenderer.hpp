#include "IRenderer.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "pipeline/IPipeline.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "renderer/RendererConfig.hpp"

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::pipe {
   class IShaderCompiler;
}

namespace tr::gfx::rd {

   class DefaultRenderer : public IRenderer {
    public:
      explicit DefaultRenderer(const RendererConfig& config,
                               const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
                               const std::shared_ptr<pipe::IShaderCompiler>& shaderCompiler);
      ~DefaultRenderer() override = default;

      DefaultRenderer(const DefaultRenderer&) = default;
      DefaultRenderer(DefaultRenderer&&) = delete;
      auto operator=(const DefaultRenderer&) -> DefaultRenderer& = default;
      auto operator=(DefaultRenderer&&) -> DefaultRenderer& = delete;

      void bindPipeline(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) override;

      void applyShaderBinding(
          const std::unique_ptr<sb::ShaderBinding>& binding,
          uint32_t setIndex,
          const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) override;

      void render(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer,
                  std::span<cm::gpu::MeshData> meshDataList,
                  const std::tuple<vk::Viewport, vk::Rect2D>& vpScissor) override;

    private:
      std::shared_ptr<pipe::IPipeline> pipeline;
   };
}
