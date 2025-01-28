#include "BufferManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "mem/Allocator.hpp"
#include "mem/Buffer.hpp"

namespace tr {

BufferManager::BufferManager(std::shared_ptr<Allocator> newAllocator,
                             std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext)
    : allocator{std::move(newAllocator)},
      immediateTransferContext{std::move(newImmediateTransferContext)} {
}

auto BufferManager::createBuffer(size_t size,
                                 vk::Flags<vk::BufferUsageFlagBits> flags,
                                 std::string_view name,
                                 vma::MemoryUsage usage,
                                 vk::MemoryPropertyFlags memoryProperties,
                                 bool mapped) -> BufferHandle {
  const auto bufferCreateInfo = vk::BufferCreateInfo{.size = size, .usage = flags};

  auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = usage, .requiredFlags = memoryProperties};

  if (mapped) {
    allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eMapped;
  }

  auto key = bufferMapKeygen.getKey();

  bufferMap.emplace(key, allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, name));
  return key;
}

auto BufferManager::createGpuVertexBuffer(size_t size, std::string_view name) -> BufferHandle {
  const auto key = bufferMapKeygen.getKey();
  bufferMap.emplace(key, allocator->createGpuVertexBuffer(size, name));
  return key;
}

auto BufferManager::createGpuIndexBuffer(size_t size, std::string_view name) -> BufferHandle {
  const auto key = bufferMapKeygen.getKey();
  bufferMap.emplace(key, allocator->createGpuIndexBuffer(size, name));
  return key;
}

auto BufferManager::createIndirectBuffer(size_t size) -> BufferHandle {
  const auto bufferCreateInfo =
      vk::BufferCreateInfo{.size = size, .usage = vk::BufferUsageFlagBits::eIndirectBuffer};

  constexpr auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  const auto key = bufferMapKeygen.getKey();

  bufferMap.emplace(
      key,
      allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, "Buffer-IndirectDraw"));
  return key;
}

[[nodiscard]] auto BufferManager::resizeBuffer(BufferHandle handle, size_t newSize)
    -> BufferHandle {
  ZoneNamedN(var, "Resize Buffer", true);
  auto& oldBuffer = bufferMap.at(handle);

  auto bci = oldBuffer->getBufferCreateInfo();
  const auto oldSize = bci.size;
  bci.size = newSize;

  auto aci = oldBuffer->getAllocationCreateInfo();

  auto newBuffer = allocator->createBuffer(&bci, &aci);

  immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
    ZoneNamedN(var, "Copy Buffer", true);
    const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = oldSize};
    cmd.copyBuffer(oldBuffer->getBuffer(), newBuffer->getBuffer(), vbCopy);
  });

  bufferMap.erase(handle);
  const auto newHandle = bufferMapKeygen.getKey();
  bufferMap.emplace(newHandle, std::move(newBuffer));
  return newHandle;
}

[[nodiscard]] auto BufferManager::getBuffer(BufferHandle handle) const -> Buffer& {
  return *bufferMap.at(handle);
}

}
