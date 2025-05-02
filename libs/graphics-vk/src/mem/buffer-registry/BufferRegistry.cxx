#include "mem/buffer-registry/BufferRegistry.hpp"
#include "IBufferManager.hpp"

namespace tr {

BufferRegistry::BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager)
    : bufferManager{std::move(newBufferManager)} {
}

auto BufferRegistry::getOrCreate(const BufferKey& key) -> BufferHandle {
  if (bufferHandles.contains(key)) {
    return bufferHandles.at(key);
  }

  switch (key.profile.kind) {
    case BufferKind::GpuBufferRegion:
    case BufferKind::ObjectCount:
    case BufferKind::ObjectData:
    case BufferKind::ObjectDataIndex:
    case BufferKind::Material:
    case BufferKind::IndirectCommand: {
      const auto bufferCreateInfo = fromProfile(key.profile);
      const auto handle = bufferManager->createBuffer(bufferCreateInfo);
      bufferHandles.emplace(key, handle);
      return handle;
    }
    case BufferKind::Geometry: {
      const auto bufferCreateInfo = fromProfileGeometry(key.profile);
      const auto handle = bufferManager->createArenaGeometryBuffer(bufferCreateInfo);
      bufferHandles.emplace(key, handle);
      return handle;
    }
  }
}

auto BufferRegistry::fromProfile(const BufferUsageProfile& profile) -> BufferCreateInfo {
  vk::BufferUsageFlags usage{};
  if (std::ranges::contains(profile.usages, BufferUsage::Storage)) {
    usage |= vk::BufferUsageFlagBits::eStorageBuffer;
  }
  if (std::ranges::contains(profile.usages, BufferUsage::Indirect)) {
    usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
  }
  if (std::ranges::contains(profile.usages, BufferUsage::GeometryVertex)) {
    usage |= vk::BufferUsageFlagBits::eVertexBuffer;
  }
  if (std::ranges::contains(profile.usages, BufferUsage::GeometryIndex)) {
    usage |= vk::BufferUsageFlagBits::eIndexBuffer;
  }
  if (std::ranges::contains(profile.usages, BufferUsage::ShaderDeviceAddress)) {
    usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
  }

  vma::MemoryUsage memUsage = std::ranges::contains(profile.usages, BufferUsage::CpuWritable)
                                  ? vma::MemoryUsage::eCpuToGpu
                                  : vma::MemoryUsage::eGpuOnly;

  vk::MemoryPropertyFlags memProps = std::ranges::contains(profile.usages, BufferUsage::CpuWritable)
                                         ? vk::MemoryPropertyFlagBits::eHostCoherent
                                         : vk::MemoryPropertyFlagBits{};

  return BufferCreateInfo{
      .size = profile.stride * profile.maxElements,
      .flags = usage,
      .memoryUsage = memUsage,
      .memoryProperties = memProps,
      .mapped = false,
  };
}

auto BufferRegistry::fromProfileGeometry(const BufferUsageProfile& profile)
    -> ArenaGeometryBufferCreateInfo {
  const auto indexSize = profile.getExtraAs<GeometryExtraProfile>()->indexSize;
  const auto vertexSize = profile.getExtraAs<GeometryExtraProfile>()->vertexFormat.stride;
  const auto createInfo = ArenaGeometryBufferCreateInfo{.vertexSize = vertexSize,
                                                        .indexSize = indexSize,
                                                        .bufferName = profile.debugName};
  return createInfo;
}

}
