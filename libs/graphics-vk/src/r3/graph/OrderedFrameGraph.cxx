#include "OrderedFrameGraph.hpp"
#include "buffers/BufferSystem.hpp"
#include "img/ImageManager.hpp"
#include "r3/graph/BarrierGenerator.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "task/Frame.hpp"
#include "vk/command-buffer/CommandBufferRequest.hpp"

namespace tr {

OrderedFrameGraph::OrderedFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                     std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                                     std::shared_ptr<ImageManager> newImageManager,
                                     std::shared_ptr<BufferSystem> newBufferSystem)
    : commandBufferManager{std::move(newCommandBufferManager)},
      aliasRegistry{std::move(newAliasRegistry)},
      imageManager{std::move(newImageManager)},
      bufferSystem{std::move(newBufferSystem)} {
}

auto OrderedFrameGraph::addPass(std::unique_ptr<IRenderPass>&& pass) -> void {
  const auto size = renderPasses.size();
  const auto passId = pass->getId();
  renderPasses.push_back(std::move(pass));
  passesById.emplace(passId, size);
}

auto OrderedFrameGraph::getPass(PassId id) -> std::unique_ptr<IRenderPass>& {
  assert(passesById.contains(id));
  return renderPasses[passesById.at(id)];
}

auto OrderedFrameGraph::bake() -> void {
  auto barrierGenerator = BarrierGenerator{};
  barrierPlan = std::make_unique<BarrierPlan>(barrierGenerator.build(renderPasses));
}

auto OrderedFrameGraph::execute(const Frame* frame) -> FrameGraphResult {
  auto result = FrameGraphResult{};

  for (const auto& renderPass : renderPasses) {
    const auto passId = renderPass->getId();

    auto imageBarriers = std::vector<vk::ImageMemoryBarrier2>{};
    if (barrierPlan->imageBarriers.contains(passId)) {
      for (const auto& imageBarrierData : barrierPlan->imageBarriers.at(passId)) {
        const auto handle =
            frame->getLogicalImage(aliasRegistry->getHandle(imageBarrierData.alias));
        const auto& image = imageManager->getImage(handle);
        imageBarriers.push_back(imageBarrierData.imageBarrier);
        imageBarriers.back().setImage(image.getImage());
      }
    }

    auto bufferBarriers = std::vector<vk::BufferMemoryBarrier2>{};
    if (barrierPlan->bufferBarriers.contains(passId)) {

      const auto visitor = [&]<typename T>(T&& arg) {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, BufferAlias>) {
          const auto logicalHandle = aliasRegistry->getHandle(arg);
          return bufferSystem->getVkBuffer(frame->getLogicalBuffer(logicalHandle));
        }
        if constexpr (std::is_same_v<U, GlobalBufferAlias>) {
          const auto handle = aliasRegistry->getHandle(arg);
          return bufferSystem->getVkBuffer(handle);
        }
      };

      for (const auto& bufferBarrierData : barrierPlan->bufferBarriers.at(passId)) {
        const auto& buffer = std::visit(visitor, bufferBarrierData.alias);
        bufferBarriers.push_back(bufferBarrierData.bufferBarrier);
        bufferBarriers.back().setBuffer(**buffer);
      }
    }

    auto dependencyInfo = vk::DependencyInfo{
        .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size()),
        .pBufferMemoryBarriers = bufferBarriers.data(),
        .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size()),
        .pImageMemoryBarriers = imageBarriers.data(),
    };

    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = passId,
                                              .queueType = QueueType::Graphics};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    commandBuffer.begin(vk::CommandBufferBeginInfo{});
    commandBuffer.pipelineBarrier2(dependencyInfo);
    renderPass->execute(frame, commandBuffer);
    commandBuffer.end();
    result.commandBuffers.push_back(commandBuffer);
  }

  return result;
}

}
