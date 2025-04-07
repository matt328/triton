#pragma once

namespace tr {

class RenderPass;
class IFrameManager;
class VkResourceManager;

enum class RenderPassType : uint8_t {
  ForwardOpaque = 0,
  PostProcess,
  UI,
};

struct RenderPassCreateInfo {
  RenderPassType type;
  vk::Extent2D extent;
  std::optional<vk::Format> colorFormat = vk::Format::eR16G16B16A16Sfloat;
  std::optional<std::array<float, 4>> clearColor;
  vk::AttachmentLoadOp colorLoadOp = vk::AttachmentLoadOp::eClear;
  vk::AttachmentStoreOp colorStoreOp = vk::AttachmentStoreOp::eStore;

  std::optional<vk::Format> depthFormat;
  std::optional<float> clearDepth = 1.f;
  std::optional<uint32_t> clearStencil = 0u;
  vk::AttachmentLoadOp depthLoadOp = vk::AttachmentLoadOp::eClear;
  vk::AttachmentStoreOp depthStoreOp = vk::AttachmentStoreOp::eStore;

  std::optional<std::string> name;
};

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                    std::shared_ptr<VkResourceManager> newResourceManager);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = default;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = default;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createRenderPass(const RenderPassCreateInfo& createInfo) -> std::unique_ptr<RenderPass>;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<VkResourceManager> resourceManager;
};

}
