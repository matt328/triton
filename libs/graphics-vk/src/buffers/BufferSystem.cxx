#include <algorithm>

#include "BufferSystem.hpp"
#include "FrameState.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IFrameManager.hpp"
#include "mem/Allocator.hpp"
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
      allocatorMap.emplace(handle, std::make_unique<LinearAllocator>(createInfo.initialSize));
      break;
    case AllocationStrategy::Arena:
      allocatorMap.emplace(handle, std::make_unique<ArenaAllocator>(createInfo.initialSize));
      break;
    case AllocationStrategy::None:
      break;
  }
  return handle;
}

auto BufferSystem::registerPerFrameBuffer(const BufferCreateInfo& createInfo)
    -> LogicalHandle<ManagedBuffer> {
  const auto logicalHandle = bufferHandleGenerator.requestLogicalHandle();

  for (const auto& frame : frameManager->getFrames()) {
    auto newCreateInfo = createInfo;
    newCreateInfo.debugName = fmt::format("{}-Frame-{}", createInfo.debugName, frame->getIndex());
    const auto handle = registerBuffer(newCreateInfo);
    frame->addLogicalBuffer(logicalHandle, handle);
  }

  return logicalHandle;
}

// Figure out why buffer data isn't showing up
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

auto BufferSystem::allocate(Handle<ManagedBuffer> handle, size_t size)
    -> std::optional<BufferRegion> {
  assert(bufferMap.contains(handle) && "During allocation, buffer handle doesn't exist");
  std::optional<BufferRegion> maybeRegion = std::nullopt;

  const auto lifetime = bufferMap.at(handle)->lifetime;

  if (lifetime == BufferLifetime::Persistent) {
    assert(allocatorMap.contains(handle) && "No allocator found for given buffer");
    auto& bufferAllocator = allocatorMap.at(handle);
    return bufferAllocator->allocate(BufferRequest{.size = size});
  }

  Log.warn("Attempted to allocate transient buffer");

  return maybeRegion;
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
    for (const auto& rec : entry->versions | std::views::reverse) {
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
    for (const auto& rec : entry->versions | std::views::reverse) {
      if (rec->getValidFromFrame() <= frameState->getFrame() &&
          (!rec->getValidToFrame() || *rec->getValidToFrame() > frameState->getFrame())) {
        managedBuffer.emplace(rec.get());
      }
    }
  }
  return managedBuffer;
}

}
