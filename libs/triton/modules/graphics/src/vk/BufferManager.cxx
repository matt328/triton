#include "BufferManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "ResourceExceptions.hpp"
#include "geo/GeometryData.hpp"
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

auto BufferManager::addToBuffer(const IGeometryData& geometryData,
                                BufferHandle vertexBufferHandle,
                                vk::DeviceSize vertexOffset,
                                BufferHandle indexBufferHandle,
                                vk::DeviceSize indexOffset) -> void {
  const auto vbSize = geometryData.getVertexDataSize();
  const auto ibSize = geometryData.getIndexDataSize();

  try {
    // Prepare Vertex Staging Buffer
    const auto vbStagingBuffer = allocator->createStagingBuffer(vbSize, "Buffer-VertexStaging");
    void* vbData = allocator->mapMemory(*vbStagingBuffer);
    memcpy(vbData, geometryData.getVertexData(), vbSize);
    allocator->unmapMemory(*vbStagingBuffer);

    // Prepare Index Staging Buffer
    const auto ibStagingBuffer = allocator->createStagingBuffer(ibSize, "Buffer-IndexStaging");
    auto* const data = allocator->mapMemory(*ibStagingBuffer);
    memcpy(data, geometryData.getIndexData(), ibSize);
    allocator->unmapMemory(*ibStagingBuffer);

    // Destination Buffer
    auto& vertexBuffer = getBuffer(vertexBufferHandle);
    auto& indexBuffer = getBuffer(indexBufferHandle);

    immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
      const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = vertexOffset, .size = vbSize};
      cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer.getBuffer(), vbCopy);
      const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = indexOffset, .size = ibSize};
      cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer.getBuffer(), copy);
    });

  } catch (const AllocationException& ex) {
    throw ResourceUploadException(
        fmt::format("Error allocating resources for geometry, {0}", ex.what()));
  }
}

auto BufferManager::addToSingleBuffer(const void* data,
                                      size_t size,
                                      BufferHandle handle,
                                      vk::DeviceSize offset) -> void {
  try {
    // TODO(matt): make staging buffers long lived instead of allocating all the time
    const auto stagingBuffer = allocator->createStagingBuffer(size, "Buffer-Staging");
    void* bufferData = allocator->mapMemory(*stagingBuffer);
    memcpy(bufferData, data, size);
    allocator->unmapMemory(*stagingBuffer);

    auto& vertexBuffer = getBuffer(handle);

    immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
      const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = offset, .size = size};
      cmd.copyBuffer(stagingBuffer->getBuffer(), vertexBuffer.getBuffer(), copy);
    });
  } catch (const AllocationException& ex) {
    throw ResourceUploadException(
        fmt::format("Error allocating resources for geometry, {0}", ex.what()));
  }
}

}
