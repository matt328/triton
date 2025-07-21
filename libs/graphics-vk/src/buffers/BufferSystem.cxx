#include "BufferSystem.hpp"
#include "FrameState.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IFrameManager.hpp"
#include "mem/Allocator.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/ArenaAllocator.hpp"
#include "resources/allocators/IBufferAllocator.hpp"
#include "resources/allocators/LinearAllocator.hpp"
#include "task/Frame.hpp"

namespace tr {

BufferSystem::BufferSystem(std::shared_ptr<IFrameManager> newFrameManager,
                           std::shared_ptr<Device> newDevice,
                           std::shared_ptr<Allocator> newAllocator,
                           std::shared_ptr<FrameState> newFramestate)
    : frameManager{std::move(newFrameManager)},
      device{std::move(newDevice)},
      allocator{std::move(newAllocator)},
      frameState{std::move(newFramestate)} {
  Log.trace("Creating BufferSystem");
}

BufferSystem::~BufferSystem() {
  Log.trace("Destroying BufferSystem");
  device->waitIdle();
  bufferMap.clear();
  Log.trace("BufferSystem Destroyed");
}

auto BufferSystem::resize(const std::shared_ptr<TransferSystem>& transferSystem,
                          const std::vector<ResizeRequest>& resizeRequests) -> void {

  struct ResizeJob {
    Handle<ManagedBuffer> handle;
    ManagedBuffer* oldBuffer;
    std::unique_ptr<ManagedBuffer> newBuffer;
    size_t newSize;
  };

  std::vector<ResizeJob> jobs;
  std::vector<std::tuple<ManagedBuffer*, ManagedBuffer*>> copyPairs;

  // Prepare resizes
  for (const auto& resize : resizeRequests) {
    auto handle = resize.bufferHandle;
    if (!bufferMap.contains(handle)) {
      continue;
    }

    auto oldBuffer = getCurrentManagedBuffer(handle);
    if (!oldBuffer) {
      Log.warn("No current buffer for handle={}", handle.id);
      continue;
    }

    const auto bci = oldBuffer.value()->getMeta().bufferCreateInfo;
    const auto aci = oldBuffer.value()->getMeta().allocationCreateInfo;

    auto newBuffer = allocator->createBuffer2(bci, aci);
    copyPairs.emplace_back(*oldBuffer, newBuffer.get());

    jobs.push_back(ResizeJob{.handle = handle,
                             .oldBuffer = *oldBuffer,
                             .newBuffer = std::move(newBuffer),
                             .newSize = resize.newSize});
  }

  transferSystem->copyBuffers(copyPairs);

  // Finalize buffer version
  const auto currentFrame = frameState->getFrame();
  for (auto& job : jobs) {
    job.newBuffer->setValidFromFrame(currentFrame + 1);
    job.oldBuffer->setValidToFrame(currentFrame + 1);

    auto& entry = bufferMap.at(job.handle);
    entry->versions.emplace_back(std::move(job.newBuffer));
    entry->currentSize = job.newSize;
    if (allocatorMap.contains(job.handle)) {
      allocatorMap.at(job.handle)->notifyBufferResized(job.newSize);
    } else {
      Log.warn("Resized a buffer with no allocator: {}", job.handle.id);
    }
  }
}

auto BufferSystem::getBufferAddress(Handle<ManagedBuffer> handle) -> std::optional<uint64_t> {
  const auto managedBuffer = getCurrentManagedBufferConst(handle);

  return managedBuffer.transform([this](const ManagedBuffer* mb) -> uint64_t {
    return device->getVkDevice().getBufferAddress(
        vk::BufferDeviceAddressInfo{.buffer = mb->getVkBuffer()});
  });
}

auto BufferSystem::getVkBuffer(Handle<ManagedBuffer> handle) -> std::optional<const vk::Buffer*> {
  const auto managedBuffer = getCurrentManagedBufferConst(handle);

  return managedBuffer.transform(
      [](const ManagedBuffer* mb) -> const vk::Buffer* { return &mb->getVkBuffer(); });
}

auto BufferSystem::registerBuffer(const BufferCreateInfo& createInfo) -> Handle<ManagedBuffer> {
  const auto handle = bufferHandleGenerator.requestHandle();

  auto [bci, aci] = fromCreateInfo(createInfo);

  auto versions = std::deque<std::unique_ptr<ManagedBuffer>>{};
  versions.emplace_back(allocator->createBuffer2(bci, aci, createInfo.debugName));
  bufferMap.emplace(
      handle,
      std::make_unique<BufferEntry>(BufferEntry{.lifetime = createInfo.bufferLifetime,
                                                .versions = std::move(versions),
                                                .currentSize = createInfo.initialSize}));
  switch (createInfo.allocationStrategy) {
    case AllocationStrategy::Linear:
      allocatorMap.emplace(
          handle,
          std::make_unique<LinearAllocator>(handle, createInfo.initialSize, createInfo.debugName));
      break;
    case AllocationStrategy::Arena:
      allocatorMap.emplace(
          handle,
          std::make_unique<ArenaAllocator>(handle, createInfo.initialSize, createInfo.debugName));
      break;
    case AllocationStrategy::Resizable:

      break;
  }
  return handle;
}

auto BufferSystem::registerPerFrameBuffer(const BufferCreateInfo& createInfo)
    -> LogicalHandle<ManagedBuffer> {
  const auto logicalHandle = bufferHandleGenerator.requestLogicalHandle();

  for (const auto& frame : frameManager->getFrames()) {
    auto newCreateInfo = createInfo;
    newCreateInfo.debugName = std::format("{}-Frame-{}", createInfo.debugName, frame->getIndex());
    const auto handle = registerBuffer(newCreateInfo);
    frame->addLogicalBuffer(logicalHandle, handle);
  }

  return logicalHandle;
}

auto BufferSystem::insert(Handle<ManagedBuffer> handle,
                          void* data,
                          const BufferRegion& targetRegion) -> std::optional<BufferRegion> {
  std::optional<BufferRegion> maybeBufferRegion = std::nullopt;
  auto managedBuffer = getCurrentManagedBuffer(handle);
  const auto lifetime = bufferMap.at(handle)->lifetime;

  if (lifetime == BufferLifetime::Transient) {
    if (managedBuffer.has_value()) {
      managedBuffer.value()->uploadData(data, targetRegion.size, targetRegion.offset);
      maybeBufferRegion.emplace(targetRegion.offset, targetRegion.size);
      return maybeBufferRegion;
    }
  }
  Log.warn("Attempted to insert into Persistent Buffer");
  return maybeBufferRegion;
}

auto BufferSystem::allocate(Handle<ManagedBuffer> handle, size_t size) -> BufferRegion {
  assert(bufferMap.contains(handle) && "During allocation, buffer handle doesn't exist");

  const auto lifetime = bufferMap.at(handle)->lifetime;

  if (lifetime == BufferLifetime::Persistent) {
    assert(allocatorMap.contains(handle) && "No allocator found for given buffer");
    auto& bufferAllocator = allocatorMap.at(handle);
    return bufferAllocator->allocate(BufferRequest{.size = size});
  }
  assert("Tried to allocate a transient buffer");
  return BufferRegion{};
}

auto BufferSystem::checkSize(Handle<ManagedBuffer> handle, size_t size)
    -> std::optional<ResizeRequest> {
  if (!allocatorMap.contains(handle)) {
    Log.warn("Check size of a buffer that doesn't have an allocator, handle={}", handle.id);
    return std::nullopt;
  }
  auto& allocator = allocatorMap.at(handle);
  return allocator->checkSize(BufferRequest{.size = size});
}

auto BufferSystem::removeData(Handle<ManagedBuffer> handle, const BufferRegion& region) -> void {
  assert(bufferMap.contains(handle));
  if (allocatorMap.contains(handle)) {
    allocatorMap.at(handle)->freeRegion(region);
  } else {
    Log.warn("removeData called with a buffer that has no allocator registered");
  }
}

auto BufferSystem::pruneOldVersions() -> void {
  const auto currentFrame = frameState->getFrame();

  for (auto& [handle, entry] : bufferMap) {
    if (entry->lifetime == BufferLifetime::Transient) {
      continue;
    }
    auto& versions = entry->versions;
    const auto pruneBeforeFrame = currentFrame > 3 ? currentFrame - 3 : 0;
    versions.erase(std::ranges::remove_if(versions,
                                          [&](const std::unique_ptr<ManagedBuffer>& buffer) {
                                            const auto validTo = buffer->getValidToFrame();
                                            return validTo.has_value() &&
                                                   *validTo <= pruneBeforeFrame;
                                          })
                       .begin(),
                   versions.end());
  }
}

auto BufferSystem::fromCreateInfo(const BufferCreateInfo& createInfo)
    -> std::tuple<vk::BufferCreateInfo, vma::AllocationCreateInfo> {
  auto bci = vk::BufferCreateInfo{.size = createInfo.initialSize,
                                  .usage = vk::BufferUsageFlagBits::eShaderDeviceAddress};
  auto aci = vma::AllocationCreateInfo{};

  switch (createInfo.bufferUsage) {
    case BufferUsage::Storage:
      bci.usage |= vk::BufferUsageFlagBits::eStorageBuffer;
      break;
    case BufferUsage::Transfer:
      bci.usage |= vk::BufferUsageFlagBits::eTransferSrc;
      break;
    case BufferUsage::Uniform:
      bci.usage |= vk::BufferUsageFlagBits::eUniformBuffer;
      break;
  }

  if (createInfo.bufferLifetime == BufferLifetime::Persistent) {
    bci.usage |=
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst;
    aci.setUsage(vma::MemoryUsage::eGpuOnly);
    aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);
  } else if (createInfo.bufferLifetime == BufferLifetime::Transient) {
    bci.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
    aci.setUsage(vma::MemoryUsage::eCpuToGpu);
    aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eHostCoherent |
                         vk::MemoryPropertyFlagBits::eHostVisible);
  }

  if (createInfo.indirect) {
    bci.usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
  }

  return {bci, aci};
}

auto BufferSystem::getCurrentManagedBufferConst(Handle<ManagedBuffer> handle) const
    -> std::optional<const ManagedBuffer*> {
  assert(bufferMap.contains(handle));
  const auto& entry = bufferMap.at(handle);
  std::optional<const ManagedBuffer*> managedBuffer = std::nullopt;

  if (entry->lifetime == BufferLifetime::Transient) {
    managedBuffer.emplace(entry->versions.front().get());
  } else {
    for (const auto& rec : entry->versions | std::ranges::views::reverse) {
      if (rec->getValidFromFrame() <= frameState->getFrame() &&
          (!rec->getValidToFrame() || *rec->getValidToFrame() > frameState->getFrame())) {
        managedBuffer.emplace(rec.get());
      }
    }
  }
  return managedBuffer;
}

auto BufferSystem::getCurrentManagedBuffer(Handle<ManagedBuffer> handle)
    -> std::optional<ManagedBuffer*> {
  assert(bufferMap.contains(handle));
  const auto& entry = bufferMap.at(handle);
  std::optional<ManagedBuffer*> managedBuffer = std::nullopt;

  if (entry->lifetime == BufferLifetime::Transient) {
    managedBuffer.emplace(entry->versions.front().get());
  } else {
    for (const auto& rec : entry->versions | std::ranges::views::reverse) {
      if (rec->getValidFromFrame() <= frameState->getFrame() &&
          (!rec->getValidToFrame() || *rec->getValidToFrame() > frameState->getFrame())) {
        managedBuffer.emplace(rec.get());
      }
    }
  }
  return managedBuffer;
}

}
