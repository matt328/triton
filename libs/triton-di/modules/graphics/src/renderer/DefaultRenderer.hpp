#include "IRenderer.hpp"
#include "renderer/RendererConfig.hpp"

namespace tr {
class IGraphicsDevice;
}

namespace tr {
class IPipeline;
class IShaderCompiler;
}

namespace tr {

class DefaultRenderer : public IRenderer {
public:
  explicit DefaultRenderer(const RendererConfig& config,
                           std::shared_ptr<IGraphicsDevice> graphicsDevice,
                           const std::shared_ptr<IShaderCompiler>& shaderCompiler);
  ~DefaultRenderer() override = default;

  DefaultRenderer(const DefaultRenderer&) = default;
  DefaultRenderer(DefaultRenderer&&) = delete;
  auto operator=(const DefaultRenderer&) -> DefaultRenderer& = default;
  auto operator=(DefaultRenderer&&) -> DefaultRenderer& = delete;

  void bindPipeline(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) override;

  void applyShaderBinding(const std::unique_ptr<ShaderBinding>& binding,
                          uint32_t setIndex,
                          const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) override;

  void render(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer,
              std::span<::MeshData> meshDataList,
              const std::tuple<vk::Viewport, vk::Rect2D>& vpScissor) override;

private:
  std::shared_ptr<IGraphicsDevice> graphicsDevice;
  std::shared_ptr<IPipeline> pipeline;
};
}
