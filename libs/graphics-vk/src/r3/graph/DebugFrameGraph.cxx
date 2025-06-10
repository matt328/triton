#include "DebugFrameGraph.hpp"
#include "r3/graph/DirectedGraph.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "vk/core/Swapchain.hpp"

namespace tr {

DebugFrameGraph::DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                 std::shared_ptr<Swapchain> newSwapchain)
    : commandBufferManager{std::move(newCommandBufferManager)}, swapchain{std::move(newSwapchain)} {
}

DebugFrameGraph::~DebugFrameGraph() {
  passes.clear();
}

auto DebugFrameGraph::addPass(std::unique_ptr<IRenderPass>&& pass) -> void {
  passes.emplace(pass->getId(), std::move(pass));
}

auto DebugFrameGraph::getPass(PassId id) -> std::unique_ptr<IRenderPass>& {
  assert(passes.contains(id));
  return passes.at(id);
}

auto DebugFrameGraph::bake() -> void {
  // Use a DAG to order passes so that writes
  auto graph = DirectedGraph<PassId>{};
  auto lastWriter = std::unordered_map<ResourceAlias, PassId>{};
  for (const auto& [id, pass] : passes) {
    const auto& info = pass->getGraphInfo();

    const auto process = [&](const auto& usageList, bool isWrite) {
      for (const auto& usage : usageList) {
        ResourceAlias alias = usage.alias;

        if (isWrite) {
          if (lastWriter.contains(alias)) {
            graph.addEdge(lastWriter[alias], id);
          }
          lastWriter[alias] = id;
        } else {
          if (lastWriter.contains(alias)) {
            graph.addEdge(lastWriter[alias], id);
          }
        }
      }
    };

    process(info.imageReads, false);
    process(info.imageWrites, true);
    process(info.bufferReads, false);
    process(info.bufferWrites, true);
  }

  auto sortedPasses = graph.topologicalSort();

  auto imageStates = std::unordered_map<ResourceAlias, ImageState>{};
  auto bufferStates = std::unordered_map<ResourceAlias, BufferState>{};

  auto processImageUsage =
      [&](const auto& usages, auto& stateMap, auto& barrierVec, auto&& makeBarrier) {
        for (const auto& usage : usages) {
          const auto alias = usage.alias;
          const auto newLayout = usage.layout;
          const auto newAccess = usage.accessFlags;
          const auto newStage = usage.stageFlags;

          auto it = stateMap.find(alias);
          if (it != stateMap.end()) {
            const auto& prev = it->second;
            if (prev.layout != newLayout || prev.accessFlags != newAccess ||
                prev.stageFlags != newStage) {
              barrierVec.push_back(makeBarrier(alias, prev, usage));
            }
          }
          stateMap[alias] =
              ImageState{.layout = newLayout, .accessFlags = newAccess, .stageFlags = newStage};
        }
      };

  auto processBufferUsage =
      [&](const auto& usages, auto& stateMap, auto& barrierVec, auto&& makeBarrier) {
        for (const auto& usage : usages) {
          const auto alias = usage.alias;
          const auto newAccess = usage.accessFlags;
          const auto newStage = usage.stageFlags;

          auto it = stateMap.find(alias);
          if (it != stateMap.end()) {
            const auto& prev = it->second;
            if (prev.accessFlags != newAccess || prev.stageFlags != newStage) {
              barrierVec.push_back(makeBarrier(alias, prev, usage));
            }
          }
          stateMap[alias] = BufferState{.accessFlags = newAccess, .stageFlags = newStage};
        }
      };

  for (const auto passId : sortedPasses) {
    const auto& pass = *passes.at(passId);
    const auto& info = pass.getGraphInfo();

    // These need to be maps to include alias, so the actual images and buffers can be looked up and
    // filled in at render time
    // These two maps along with sorted passes i think make up the state produced by the bake()
    // method
    auto imageBarriers = std::vector<vk::ImageMemoryBarrier2>{};
    auto bufferBarriers = std::vector<vk::BufferMemoryBarrier2>{};

    processImageUsage(info.imageReads,
                      imageStates,
                      imageBarriers,
                      [](ResourceAlias, const ImageState& prev, const ImageUsageInfo& usage) {
                        return vk::ImageMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                       .srcAccessMask = prev.accessFlags,
                                                       .dstStageMask = usage.stageFlags,
                                                       .dstAccessMask = usage.accessFlags,
                                                       .oldLayout = prev.layout,
                                                       .newLayout = usage.layout,
                                                       .image = nullptr,
                                                       .subresourceRange = {}};
                      });

    processImageUsage(info.imageWrites,
                      imageStates,
                      imageBarriers,
                      [](ResourceAlias, const ImageState& prev, const ImageUsageInfo& usage) {
                        return vk::ImageMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                       .srcAccessMask = prev.accessFlags,
                                                       .dstStageMask = usage.stageFlags,
                                                       .dstAccessMask = usage.accessFlags,
                                                       .oldLayout = prev.layout,
                                                       .newLayout = usage.layout,
                                                       .image = nullptr,
                                                       .subresourceRange = {}};
                      });

    processBufferUsage(info.bufferReads,
                       bufferStates,
                       bufferBarriers,
                       [](ResourceAlias, const BufferState& prev, const BufferUsageInfo& usage) {
                         return vk::BufferMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                         .srcAccessMask = prev.accessFlags,
                                                         .dstStageMask = usage.stageFlags,
                                                         .dstAccessMask = usage.accessFlags,
                                                         .buffer = nullptr, // set later
                                                         .offset = 0,
                                                         .size = VK_WHOLE_SIZE};
                       });

    processBufferUsage(info.bufferWrites,
                       bufferStates,
                       bufferBarriers,
                       [](ResourceAlias, const BufferState& prev, const BufferUsageInfo& usage) {
                         return vk::BufferMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                         .srcAccessMask = prev.accessFlags,
                                                         .dstStageMask = usage.stageFlags,
                                                         .dstAccessMask = usage.accessFlags,
                                                         .buffer = nullptr,
                                                         .offset = 0,
                                                         .size = VK_WHOLE_SIZE};
                       });

    // Store Barriers
    auto dependencyInfo = vk::DependencyInfo{
        .memoryBarrierCount = 0,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size()),
        .pBufferMemoryBarriers = bufferBarriers.data(),
        .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size()),
        .pImageMemoryBarriers = imageBarriers.data(),
    };
  }
}

auto DebugFrameGraph::execute(const Frame* frame) -> FrameGraphResult {
  ZoneScopedN("DebugFrameGraph::execute");
  auto frameGraphResult = FrameGraphResult{};

  // for (const auto& pass : computePasses | std::views::values) {
  //   const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
  //                                             .frameId = frame->getIndex(),
  //                                             .passId = pass->getId(),
  //                                             .queueType = QueueType::Graphics};
  //   auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
  //   pass->dispatch(frame, commandBuffer);
  //   frameGraphResult.commandBuffers.push_back(commandBuffer);
  // }

  // for (const auto& pass : graphicsPasses | std::views::values) {
  //   const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
  //                                             .frameId = frame->getIndex(),
  //                                             .passId = pass->getId(),
  //                                             .queueType = QueueType::Graphics};
  //   auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
  //   commandBuffer.begin(
  //       vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  //   pass->execute(frame, commandBuffer);

  //   auto swapchainImage = swapchain->getSwapchainImage(frame->getSwapchainImageIndex());

  //   vk::ImageSubresourceRange fullRange{
  //       .aspectMask = vk::ImageAspectFlagBits::eColor,
  //       .baseMipLevel = 0,
  //       .levelCount = 1, // replace with actual mip levels
  //       .baseArrayLayer = 0,
  //       .layerCount = 1 // replace with actual array layers
  //   };

  //   vk::ImageMemoryBarrier2 presentBarrier{
  //       .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
  //       .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
  //       .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
  //       .dstAccessMask = {},
  //       .oldLayout = vk::ImageLayout::eUndefined,
  //       .newLayout = vk::ImageLayout::ePresentSrcKHR,
  //       .image = swapchainImage,
  //       .subresourceRange = fullRange};

  //   vk::DependencyInfo info{.imageMemoryBarrierCount = 1, .pImageMemoryBarriers =
  //   &presentBarrier};

  //   commandBuffer.pipelineBarrier2(info);
  //   commandBuffer.end();
  //   frameGraphResult.commandBuffers.push_back(commandBuffer);
  // }
  return frameGraphResult;
}

}
