#include "BufferSystem.hpp"
#include "buffers/ArenaStrategy.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/HostVisibleStrategy.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IFrameManager.hpp"
#include "mem/Allocator.hpp"
#include "task/Frame.hpp"

namespace tr {

BufferSystem::BufferSystem(std::shared_ptr<IFrameManager> newFrameManager,
                           std::shared_ptr<Device> newDevice,
                           std::shared_ptr<Allocator> newAllocator)
    : frameManager{std::move(newFrameManager)},
      device{std::move(newDevice)},
      allocator{std::move(newAllocator)} {

  const auto hostVisibleHandle = strategyHandleGenerator.requestHandle();
  strategyMap.emplace(hostVisibleHandle, std::make_unique<HostVisibleStrategy>());

  const auto arenaHandle = strategyHandleGenerator.requestHandle();
  strategyMap.emplace(arenaHandle, std::make_unique<ArenaStrategy>());
}

auto BufferSystem::registerBuffer(BufferCreateInfo createInfo) -> Handle<ManagedBuffer> {
  const auto handle = bufferHandleGenerator.requestHandle();

  auto [bci, aci] = fromCreateInfo(createInfo);
  bufferMap.emplace(handle, allocator->createBuffer2(&bci, &aci));

  return handle;
}

auto BufferSystem::registerPerFrameBuffer(BufferCreateInfo createInfo)
    -> LogicalHandle<ManagedBuffer> {
  const auto logicalHandle = bufferHandleGenerator.requestLogicalHandle();

  for (const auto& frame : frameManager->getFrames()) {
    const auto handle = registerBuffer(createInfo);
    frame->addLogicalBuffer(logicalHandle, handle);
  }

  return logicalHandle;
}

auto BufferSystem::rewrite(Handle<ManagedBuffer> handle, const void* data, size_t size) -> void {
  auto& buffer = bufferMap.at(handle);
  auto& strategy = strategyMap.at(buffer->getMeta().strategyHandle);
  strategy->rewrite(*buffer, data, size);
}

auto BufferSystem::insert(Handle<ManagedBuffer> handle, const void* data, size_t size)
    -> BufferRegion {
  auto& buffer = bufferMap.at(handle);
  auto& strategy = strategyMap.at(buffer->getMeta().strategyHandle);
  return strategy->insert(*buffer, data, size);
}

auto BufferSystem::removeData(Handle<ManagedBuffer> handle, const BufferRegion& region) -> void {
  auto& buffer = bufferMap.at(handle);
  auto& strategy = strategyMap.at(buffer->getMeta().strategyHandle);
  strategy->remove(*buffer, region);
}

auto BufferSystem::fromCreateInfo(const BufferCreateInfo& createInfo)
    -> std::tuple<vk::BufferCreateInfo, vma::AllocationCreateInfo> {
  auto bci = vk::BufferCreateInfo{
      .size = createInfo.initialSize,
  };
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

  if (createInfo.bufferType == BufferType::DeviceArena) {
    bci.usage |=
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst;
    aci.setUsage(vma::MemoryUsage::eGpuOnly);
    aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);
  } else if (createInfo.bufferType == BufferType::IndirectCommand) {
    bci.usage |=
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eIndirectBuffer;
    aci.setUsage(vma::MemoryUsage::eGpuOnly);
    aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);
  } else if (createInfo.bufferType == BufferType::HostTransient) {
    bci.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
    aci.setUsage(vma::MemoryUsage::eCpuToGpu);
    aci.setRequiredFlags(vk::MemoryPropertyFlagBits::eHostCoherent);
  }

  return {bci, aci};
}
}
