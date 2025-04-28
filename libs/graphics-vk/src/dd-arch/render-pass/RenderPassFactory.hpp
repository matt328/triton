#pragma once

#include "bk/HandleGenerator.hpp"
#include "dd/DrawContext.hpp"
#include "dd/RenderConfig.hpp"
#include "dd/render-pass/GraphicsPass.hpp"
#include "gfx/IFrameManager.hpp"
#include "pipeline/IShaderModuleFactory.hpp"
#include "vk/core/Device.hpp"
#include "dd/render-pass/ComputePass.hpp"

namespace tr {

struct CullingPassInfo {
  Handle<ComputePass> handle;
  BufferHandle objectData;
};

struct GeometryPassInfo {
  Handle<GraphicsPass> handle;
};

struct LightingPassInfo {
  Handle<GraphicsPass> handle;
};

struct CompositePassInfo {
  Handle<GraphicsPass> handle;
};

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                    std::shared_ptr<ImageManager> newImageManager,
                    std::shared_ptr<IShaderModuleFactory> newShaderModuleFactory,
                    std::shared_ptr<Device> newDevice);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = delete;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = delete;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createCullingPass() -> CullingPassInfo;
  auto createGeometryPass() -> GeometryPassInfo;
  auto createLightingPass() -> LightingPassInfo;
  auto createCompositePass() -> CompositePassInfo;

  auto registerDrawContext(const RenderConfig& renderConfig, DrawContext* drawContext) -> void;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<IShaderModuleFactory> shaderModuleFactory;
  std::shared_ptr<Device> device;

  HandleGenerator<ComputePass> computePassGenerator;
  HandleGenerator<GraphicsPass> graphicsPassGenerator;

  std::unordered_map<Handle<ComputePass>, std::unique_ptr<ComputePass>> computePassMap;
  std::unordered_map<Handle<GraphicsPass>, std::unique_ptr<GraphicsPass>> graphicsPassMap;
};

}
