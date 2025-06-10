#pragma once

#include "gfx/IFrameGraph.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "task/Frame.hpp"

namespace tr {

class Swapchain;
class ResourceAliasRegistry;
class ImageManager;
class BufferSystem;

struct ImageState {
  vk::ImageLayout layout;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
};

struct BufferState {
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
};

struct ImageBarrierData {
  vk::ImageMemoryBarrier2 imageBarrier;
  ImageAlias alias;
};

struct BufferBarrierData {
  vk::BufferMemoryBarrier2 bufferBarrier;
  BufferAlias alias;
};

class DebugFrameGraph : public IFrameGraph {
public:
  explicit DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                           std::shared_ptr<Swapchain> newSwapchain,
                           std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                           std::shared_ptr<ImageManager> newImageManager,
                           std::shared_ptr<BufferSystem> newBufferSystem);
  ~DebugFrameGraph() override;

  DebugFrameGraph(const DebugFrameGraph&) = default;
  DebugFrameGraph(DebugFrameGraph&&) = delete;
  auto operator=(const DebugFrameGraph&) -> DebugFrameGraph& = default;
  auto operator=(DebugFrameGraph&&) -> DebugFrameGraph& = delete;

  auto addPass(std::unique_ptr<IRenderPass>&& pass) -> void override;

  [[nodiscard]] auto getPass(PassId id) -> std::unique_ptr<IRenderPass>& override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> FrameGraphResult override;

private:
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<BufferSystem> bufferSystem;

  std::unordered_map<PassId, std::unique_ptr<IRenderPass>> passes;

  std::vector<PassId> sortedPasses{};
  std::unordered_map<PassId, std::vector<ImageBarrierData>> imageBarriers;
  std::unordered_map<PassId, std::vector<BufferBarrierData>> bufferBarriers;
};

}
