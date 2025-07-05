#pragma once

#include "bk/HandleGenerator.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/render-pass/CreateInfo.hpp"

namespace tr {

class GraphicsPass;
class ComputePass;
class PipelineFactory;
class ImageManager;
class IFrameManager;
class ManagedImage;
class ContextFactory;
class IRenderPass;
class ResourceAliasRegistry;
class IShaderBindingFactory;
class DSLayoutManager;
class Device;
class IWindow;
class Instance;
class PhysicalDevice;
namespace queue {
class Graphics;
}

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                    std::shared_ptr<ImageManager> newImageManager,
                    std::shared_ptr<IFrameManager> newFrameManager,
                    std::shared_ptr<ContextFactory> newDrawContextFactory,
                    std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                    std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                    std::shared_ptr<DSLayoutManager> newLayoutManager,
                    std::shared_ptr<Device> newDevice,
                    std::shared_ptr<IWindow> newWindow,
                    std::shared_ptr<Instance> newInstance,
                    std::shared_ptr<PhysicalDevice> newPhysicalDevice,
                    std::shared_ptr<queue::Graphics> newGraphicsQueue);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = delete;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = delete;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createRenderPass(RenderPassCreateInfo createInfo) -> std::unique_ptr<IRenderPass>;

private:
  std::shared_ptr<PipelineFactory> pipelineFactory;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<ContextFactory> drawContextFactory;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<DSLayoutManager> layoutManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<Instance> instance;
  std::shared_ptr<PhysicalDevice> physicalDevice;
  std::shared_ptr<queue::Graphics> graphicsQueue;

  HandleGenerator<ManagedImage> imageHandleGenerator;

  auto createForwardPass(PassId passId, const ForwardPassCreateInfo& createInfo)
      -> std::unique_ptr<IRenderPass>;
  auto createCullingPass(PassId passId, CullingPassCreateInfo createInfo)
      -> std::unique_ptr<IRenderPass>;
  auto createCompositionPass(PassId passId, CompositionPassCreateInfo createInfo)
      -> std::unique_ptr<IRenderPass>;
  auto createImGuiPass(PassId passId, ImGuiPassCreateInfo createInfo)
      -> std::unique_ptr<IRenderPass>;
};

}
