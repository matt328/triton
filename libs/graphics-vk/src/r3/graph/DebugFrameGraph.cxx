#include "DebugFrameGraph.hpp"
#include "buffers/BufferSystem.hpp"
#include "img/ImageManager.hpp"
#include "r3/graph/DirectedGraph.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "vk/core/Swapchain.hpp"

namespace tr {

DebugFrameGraph::DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                 std::shared_ptr<Swapchain> newSwapchain,
                                 std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                                 std::shared_ptr<ImageManager> newImageManager,
                                 std::shared_ptr<BufferSystem> newBufferSystem)
    : commandBufferManager{std::move(newCommandBufferManager)},
      swapchain{std::move(newSwapchain)},
      aliasRegistry{std::move(newAliasRegistry)},
      imageManager{std::move(newImageManager)},
      bufferSystem{std::move(newBufferSystem)} {
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

/// Updates sortedPasses and imageBarriers and bufferBarriers
/// Relatively expensive and sloppy, so don't call every frame
auto DebugFrameGraph::bake() -> void {
  // Use a DAG to order passes so that writes
  auto graph = DirectedGraph<PassId>{};
  auto lastWriter = std::unordered_map<ImageAlias, PassId>{};
  auto lastWriterBuffers = std::unordered_map<BufferAlias, PassId>{};
  for (const auto& [id, pass] : passes) {
    const auto& info = pass->getGraphInfo();

    const auto processImages = [&](const auto& usageList, bool isWrite) {
      for (const auto& usage : usageList) {
        auto alias = usage.alias;

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

    const auto processBuffers = [&](const auto& usageList, bool isWrite) {
      for (const auto& usage : usageList) {
        auto alias = usage.alias;

        if (isWrite) {
          if (lastWriterBuffers.contains(alias)) {
            graph.addEdge(lastWriterBuffers[alias], id);
          }
          lastWriterBuffers[alias] = id;
        } else {
          if (lastWriterBuffers.contains(alias)) {
            graph.addEdge(lastWriterBuffers[alias], id);
          }
        }
      }
    };

    // Need to split out separate process functions for images and buffers
    processImages(info.imageReads, false);
    processImages(info.imageWrites, true);
    processBuffers(info.bufferReads, false);
    processBuffers(info.bufferWrites, true);
  }

  auto imageStates = std::unordered_map<ImageAlias, ImageState>{};
  auto bufferStates = std::unordered_map<BufferAlias, BufferState>{};

  auto processImageUsage =
      [&](PassId passId, const auto& usages, auto& stateMap, auto& barrierVec, auto&& makeBarrier) {
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
              barrierVec[passId].push_back(makeBarrier(alias, prev, usage));
            }
          }
          stateMap[alias] =
              ImageState{.layout = newLayout, .accessFlags = newAccess, .stageFlags = newStage};
        }
      };

  auto processBufferUsage = [&](PassId passId,
                                const std::vector<BufferUsageInfo>& usages,
                                auto& stateMap,
                                auto& barrierVec,
                                auto&& makeBarrier) {
    for (const auto& usage : usages) {
      const auto alias = usage.alias;
      const auto newAccess = usage.accessFlags;
      const auto newStage = usage.stageFlags;

      auto it = stateMap.find(alias);
      if (it != stateMap.end()) {
        const auto& prev = it->second;
        if (prev.accessFlags != newAccess || prev.stageFlags != newStage) {
          barrierVec[passId].push_back(makeBarrier(alias, prev, usage));
        }
      }
      stateMap[alias] = BufferState{.accessFlags = newAccess, .stageFlags = newStage};
    }
  };

  for (const auto passId : sortedPasses) {
    const auto& pass = *passes.at(passId);
    const auto& info = pass.getGraphInfo();

    processImageUsage(passId,
                      info.imageReads,
                      imageStates,
                      imageBarriers,
                      [](ImageAlias alias, const ImageState& prev, const ImageUsageInfo& usage) {
                        return ImageBarrierData{.imageBarrier =
                                                    vk::ImageMemoryBarrier2{
                                                        .srcStageMask = prev.stageFlags,
                                                        .srcAccessMask = prev.accessFlags,
                                                        .dstStageMask = usage.stageFlags,
                                                        .dstAccessMask = usage.accessFlags,
                                                        .oldLayout = prev.layout,
                                                        .newLayout = usage.layout,
                                                        .image = nullptr,
                                                        .subresourceRange = {},
                                                    },
                                                .alias = alias};
                      });

    processImageUsage(passId,
                      info.imageWrites,
                      imageStates,
                      imageBarriers,
                      [](ImageAlias alias, const ImageState& prev, const ImageUsageInfo& usage) {
                        return ImageBarrierData{
                            .imageBarrier =
                                vk::ImageMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                        .srcAccessMask = prev.accessFlags,
                                                        .dstStageMask = usage.stageFlags,
                                                        .dstAccessMask = usage.accessFlags,
                                                        .oldLayout = prev.layout,
                                                        .newLayout = usage.layout,
                                                        .image = nullptr,
                                                        .subresourceRange = {}},
                            .alias = alias};
                      });

    processBufferUsage(
        passId,
        info.bufferReads,
        bufferStates,
        bufferBarriers,
        [](BufferAlias alias, const BufferState& prev, const BufferUsageInfo& usage) {
          return BufferBarrierData{.bufferBarrier =
                                       vk::BufferMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                                .srcAccessMask = prev.accessFlags,
                                                                .dstStageMask = usage.stageFlags,
                                                                .dstAccessMask = usage.accessFlags,
                                                                .buffer = nullptr, // set later
                                                                .offset = 0,
                                                                .size = VK_WHOLE_SIZE},
                                   .alias = alias};
        });

    processBufferUsage(
        passId,
        info.bufferWrites,
        bufferStates,
        bufferBarriers,
        [](BufferAlias alias, const BufferState& prev, const BufferUsageInfo& usage) {
          return BufferBarrierData{.bufferBarrier =
                                       vk::BufferMemoryBarrier2{.srcStageMask = prev.stageFlags,
                                                                .srcAccessMask = prev.accessFlags,
                                                                .dstStageMask = usage.stageFlags,
                                                                .dstAccessMask = usage.accessFlags,
                                                                .buffer = nullptr,
                                                                .offset = 0,
                                                                .size = VK_WHOLE_SIZE},
                                   .alias = alias};
        });
  }
}

auto DebugFrameGraph::execute([[maybe_unused]] const Frame* frame) -> FrameGraphResult {
  ZoneScopedN("DebugFrameGraph::execute");
  auto frameGraphResult = FrameGraphResult{};

  for (const auto& passId : sortedPasses) {
    auto& pass = passes.at(passId);
    auto& imageBarrierDataList = imageBarriers.at(passId);
    auto& bufferBarrierDataList = bufferBarriers.at(passId);

    // Resolve the Aliases to Images
    auto imageBarrierList = std::vector<vk::ImageMemoryBarrier2>{};
    for (auto& ibd : imageBarrierDataList) {
      const auto handle = frame->getLogicalImage(aliasRegistry->getHandle(ibd.alias));
      const auto& image = imageManager->getImage(handle);
      ibd.imageBarrier.setImage(image.getImage());
      imageBarrierList.push_back(ibd.imageBarrier);
    }

    auto bufferBarrierList = std::vector<vk::BufferMemoryBarrier2>{};
    for (auto& bufferBarrierData : bufferBarrierDataList) {
      const auto handle =
          frame->getLogicalBuffer(aliasRegistry->getHandle(bufferBarrierData.alias));
      auto buffer = bufferSystem->getVkBuffer(handle);
      bufferBarrierData.bufferBarrier.setBuffer(**buffer);
      bufferBarrierList.push_back(bufferBarrierData.bufferBarrier);
    }

    auto dependencyInfo = vk::DependencyInfo{
        .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarrierList.size()),
        .pBufferMemoryBarriers = bufferBarrierList.data(),
        .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarrierList.size()),
        .pImageMemoryBarriers = imageBarrierList.data(),
    };

    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Graphics};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    commandBuffer.pipelineBarrier2(dependencyInfo);
    pass->execute(frame, commandBuffer);
    frameGraphResult.commandBuffers.push_back(commandBuffer);
  }
  return frameGraphResult;
}

}
