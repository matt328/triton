#pragma once

#include "dd/render-pass/RenderPassCreateInfo.hpp"

namespace tr {

class RenderPass;
class IFrameManager;
class ImageManager;
class IShaderModuleFactory;
class Device;

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                    std::shared_ptr<ImageManager> newImageManager,
                    std::shared_ptr<IShaderModuleFactory> newShaderModuleFactory,
                    std::shared_ptr<Device> newDevice);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = default;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = default;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createRenderPass(RenderPassCreateInfo& createInfo) -> std::unique_ptr<RenderPass>;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<IShaderModuleFactory> shaderModuleFactory;
  std::shared_ptr<Device> device;
};

}
