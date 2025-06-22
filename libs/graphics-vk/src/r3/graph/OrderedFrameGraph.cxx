#include "OrderedFrameGraph.hpp"
#include "buffers/BufferSystem.hpp"
#include "img/ImageManager.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/graph/barriers/BarrierBuilder.hpp"
#include "r3/graph/barriers/BarrierPrecursorGenerator.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "task/Frame.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"
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
  auto barrierPrecursorGenerator = BarrierPrecursorGenerator{};
  barrierPrecursorPlan = barrierPrecursorGenerator.build(renderPasses);
  Log.debug("{}", barrierPrecursorPlan);
}

auto OrderedFrameGraph::execute(Frame* frame) -> FrameGraphResult {
  auto result = FrameGraphResult{};

  for (const auto& renderPass : renderPasses) {
    const auto passId = renderPass->getId();

    auto imageBarriers = std::vector<vk::ImageMemoryBarrier2>{};

    if (barrierPrecursorPlan.imagePrecursors.contains(passId)) {
      for (const auto& precursor : barrierPrecursorPlan.imagePrecursors.at(passId)) {
        const auto handle = frame->getLogicalImage(aliasRegistry->getHandle(precursor.alias));

        auto lastUse = std::optional<LastImageUse>();

        if (precursor.alias == ImageAlias::SwapchainImage) {
          lastUse = getSwapchainImageLastUse(handle);
        } else {
          lastUse = frame->getLastImageUse(precursor.alias);
        }
        auto imageBarrier = BarrierBuilder::build(precursor, lastUse);
        if (imageBarrier) {

          const auto& image = imageManager->getImage(handle);
          imageBarrier->setImage(image.getImage());
          imageBarriers.push_back(*imageBarrier);
        }
        const auto newLastUse = LastImageUse{
            .accessMode = precursor.accessMode,
            .access = precursor.accessFlags,
            .stage = precursor.stageFlags,
            .layout = precursor.layout,
        };
        if (precursor.alias == ImageAlias::SwapchainImage) {
          setSwapchainImageLastUse(handle, newLastUse);
        } else {
          frame->setLastImageUse(precursor.alias, newLastUse);
        }
      }
    }

    auto bufferBarriers = std::vector<vk::BufferMemoryBarrier2>{};
    if (barrierPrecursorPlan.bufferPrecursors.contains(passId)) {

      const auto visitor = [&]<typename T>(T&& arg) {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, BufferAlias>) {
          const auto logicalHandle = aliasRegistry->getHandle(std::forward<T>(arg));
          return bufferSystem->getVkBuffer(frame->getLogicalBuffer(logicalHandle));
        }
        if constexpr (std::is_same_v<U, GlobalBufferAlias>) {
          const auto handle = aliasRegistry->getHandle(std::forward<T>(arg));
          return bufferSystem->getVkBuffer(handle);
        }
      };

      for (const auto& precursor : barrierPrecursorPlan.bufferPrecursors.at(passId)) {
        const auto& buffer = std::visit(visitor, precursor.alias);
        auto lastUse = frame->getLastBufferUse(precursor.alias);
        auto bufferBarrier = BarrierBuilder::build(precursor, lastUse);
        if (bufferBarrier) {
          bufferBarrier->setBuffer(**buffer);
          bufferBarriers.push_back(*bufferBarrier);
        }
        frame->setLastBufferUse(precursor.alias,
                                LastBufferUse{
                                    .passId = passId,
                                    .accessMask = precursor.accessFlags,
                                    .stageMask = precursor.stageFlags,
                                });
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

auto OrderedFrameGraph::getSwapchainImageLastUse(Handle<ManagedImage> handle)
    -> std::optional<LastImageUse> {
  return swapchainLastUses.contains(handle)
             ? std::make_optional<LastImageUse>(swapchainLastUses.at(handle))
             : std::nullopt;
}

auto OrderedFrameGraph::setSwapchainImageLastUse(Handle<ManagedImage> handle,
                                                 LastImageUse lastImageUse) -> void {
  swapchainLastUses.insert_or_assign(handle, lastImageUse);
}

}
