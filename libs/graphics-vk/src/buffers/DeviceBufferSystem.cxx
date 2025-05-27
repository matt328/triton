#include "DeviceBufferSystem.hpp"
#include "FrameState.hpp"
#include "buffers/DeviceBufferCreateInfo.hpp"
#include "mem/Allocator.hpp"
#include "resources/allocators/LinearAllocator.hpp"
#include "vk/core/Device.hpp"

namespace tr {

DeviceBufferSystem::DeviceBufferSystem(std::shared_ptr<Device> newDevice,
                                       std::shared_ptr<Allocator> newAllocator,
                                       std::shared_ptr<FrameState> newFrameState)
    : device{std::move(newDevice)},
      allocator{std::move(newAllocator)},
      frameState{std::move(newFrameState)} {
  Log.trace("Creating DeviceBufferSystem");
}

DeviceBufferSystem::~DeviceBufferSystem() {
  Log.trace("Destroying DeviceBufferSystem");
  bufferMap.clear();
}

auto DeviceBufferSystem::resizeBuffers(const UploadPlan& uploadPlan) -> void {
}

auto DeviceBufferSystem::registerBuffer(const DeviceBufferCreateInfo& createInfo)
    -> Handle<ManagedBuffer> {
  const auto handle = generator.requestHandle();
  if (createInfo.allocationType == AllocationType::Linear) {
    allocatorMap.emplace(handle, std::make_unique<LinearAllocator>(createInfo.initialSize));
  }
  auto [bci, aci] = fromCreateInfo(createInfo);
  const auto tempHandle = Handle<IBufferStrategy>{}; // Remove the strategy handle

  auto versions = std::deque<std::unique_ptr<ManagedBuffer>>{};
  versions.emplace_back(allocator->createBuffer2(&bci, &aci, tempHandle, createInfo.debugName));
  bufferMap.emplace(handle,
                    std::make_unique<BufferEntry>(BufferEntry{.versions = std::move(versions)}));
  return handle;
}

auto DeviceBufferSystem::tryInsert(Handle<ManagedBuffer> handle, const void* data, size_t size)
    -> std::optional<BufferRegion> {
  return std::nullopt;
}

auto DeviceBufferSystem::removeData(Handle<ManagedBuffer> handle, const BufferRegion& region)
    -> void {
}

auto DeviceBufferSystem::getBufferAddress(Handle<ManagedBuffer> handle) -> std::optional<uint64_t> {
  assert(bufferMap.contains(handle));
  const auto& entry = bufferMap.at(handle);
  for (const auto& rec : entry->versions | std::views::reverse) {
    if (rec->getValidFromFrame() <= frameState->getFrame() &&
        (!rec->getValidToFrame() || *rec->getValidToFrame() > frameState->getFrame())) {
      return device->getVkDevice().getBufferAddress(
          vk::BufferDeviceAddressInfo{.buffer = rec->getVkBuffer()});
    }
  }
  return std::nullopt;
}

auto DeviceBufferSystem::getVkBuffer(Handle<ManagedBuffer> handle) -> std::optional<vk::Buffer> {
  assert(bufferMap.contains(handle));
  const auto& entry = bufferMap.at(handle);
  for (const auto& rec : entry->versions | std::views::reverse) {
    if (rec->getValidFromFrame() <= frameState->getFrame() &&
        (!rec->getValidToFrame() || *rec->getValidToFrame() > frameState->getFrame())) {
      return rec->getVkBuffer();
    }
  }
  return std::nullopt;
}

auto DeviceBufferSystem::endFrame() -> void {
}

auto DeviceBufferSystem::fromCreateInfo(const DeviceBufferCreateInfo& createInfo)
    -> std::tuple<vk::BufferCreateInfo, vma::AllocationCreateInfo> {
  auto bci = vk::BufferCreateInfo{.size = createInfo.initialSize,
                                  .usage = vk::BufferUsageFlagBits::eShaderDeviceAddress};
  auto aci = vma::AllocationCreateInfo{};

  bci.usage |= vk::BufferUsageFlagBits::eStorageBuffer;

  bci.usage |=
      vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst;
  aci.setUsage(vma::MemoryUsage::eGpuOnly);
  aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);

  return {bci, aci};
}
}
