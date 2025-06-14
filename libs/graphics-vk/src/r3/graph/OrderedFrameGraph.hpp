#pragma once

#include "gfx/IFrameGraph.hpp"
#include "r3/graph/BarrierGenerator.hpp"

namespace tr {

class CommandBufferManager;
class ResourceAliasRegistry;
class ImageManager;
class BufferSystem;

class OrderedFrameGraph : public IFrameGraph {
public:
  OrderedFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                    std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                    std::shared_ptr<ImageManager> newImageManager,
                    std::shared_ptr<BufferSystem> newBufferSystem);
  ~OrderedFrameGraph() override = default;

  OrderedFrameGraph(const OrderedFrameGraph&) = delete;
  OrderedFrameGraph(OrderedFrameGraph&&) = delete;
  auto operator=(const OrderedFrameGraph&) -> OrderedFrameGraph& = delete;
  auto operator=(OrderedFrameGraph&&) -> OrderedFrameGraph& = delete;

  auto addPass(std::unique_ptr<IRenderPass>&& pass) -> void override;

  [[nodiscard]] auto getPass(PassId id) -> std::unique_ptr<IRenderPass>& override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> FrameGraphResult override;

private:
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<BufferSystem> bufferSystem;

  std::vector<std::unique_ptr<IRenderPass>> renderPasses;
  std::unordered_map<PassId, size_t> passesById;
  std::unique_ptr<BarrierPlan> barrierPlan;
};

}
