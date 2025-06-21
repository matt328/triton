#pragma once

#include "r3/render-pass/CreateInfo.hpp"
#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

class ImageManager;
class ContextFactory;
class ResourceAliasRegistry;
class PipelineFactory;
class IWindow;
class Instance;
class PhysicalDevice;
class Device;
namespace queue {
class Graphics;
}

class ImGuiPass : public IRenderPass {
public:
  ImGuiPass(std::shared_ptr<ImageManager> newImageManager,
            std::shared_ptr<ContextFactory> newDrawContextFactory,
            std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
            std::shared_ptr<PipelineFactory> newPipelineFactory,
            std::shared_ptr<Device> newDevice,
            const std::shared_ptr<IWindow>& window,
            const std::shared_ptr<Instance>& instance,
            const std::shared_ptr<PhysicalDevice>& physicalDevice,
            const std::shared_ptr<queue::Graphics>& graphicsQueue,
            ImGuiPassCreateInfo createInfo,
            PassId newPassId);
  ~ImGuiPass();

  ImGuiPass(const ImGuiPass&) = delete;
  ImGuiPass(ImGuiPass&&) = delete;
  auto operator=(const ImGuiPass&) -> ImGuiPass& = delete;
  auto operator=(ImGuiPass&&) -> ImGuiPass& = delete;

  [[nodiscard]] auto getId() const -> PassId override;
  auto execute(Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void override;
  auto registerDispatchContext(Handle<IDispatchContext> handle) -> void override;
  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<ContextFactory> drawContextFactory;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;
  std::shared_ptr<PipelineFactory> pipelineFactory;
  std::shared_ptr<Device> device;
  ImageAlias colorAlias;
  PassId id;

  std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;

  std::optional<vk::raii::Pipeline> pipeline;
  std::optional<vk::raii::PipelineLayout> pipelineLayout;

  vk::RenderingInfo renderingInfo;
  std::optional<vk::RenderingAttachmentInfo> colorAttachmentInfo;

  std::vector<Handle<IDispatchContext>> drawableContexts;
};

}
