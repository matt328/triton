#include "GeometryBufferPack.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "resources/allocators/ArenaAllocator.hpp"
#include "resources/allocators/IBufferAllocator.hpp"
#include "resources/allocators/LinearAllocator.hpp"

namespace tr {

GeometryBufferPack::GeometryBufferPack(std::shared_ptr<BufferSystem> newBufferSystem,
                                       const std::shared_ptr<ResourceAliasRegistry>& aliasRegistry)
    : bufferSystem{std::move(newBufferSystem)},
      indexBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.allocationStrategy = AllocationStrategy::Arena,
                           .bufferLifetime = BufferLifetime::Persistent,
                           .initialSize = IndexBufferInitialSize,
                           .itemStride = sizeof(uint32_t),
                           .debugName = "Buffer-GeometryIndex"})},
      positionBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.allocationStrategy = AllocationStrategy::Arena,
                           .bufferLifetime = BufferLifetime::Persistent,
                           .initialSize = PositionBufferInitialSize,
                           .itemStride = sizeof(glm::vec3),
                           .debugName = "Buffer-GeometryPosition"})},
      colorBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.allocationStrategy = AllocationStrategy::Arena,
                           .bufferLifetime = BufferLifetime::Persistent,
                           .initialSize = ColorBufferInitialSize,
                           .itemStride = sizeof(glm::vec4),
                           .debugName = "Buffer-GeometryColors"})},
      texCoordBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.allocationStrategy = AllocationStrategy::Arena,
                           .bufferLifetime = BufferLifetime::Persistent,
                           .initialSize = TexCoordBufferInitialSize,
                           .itemStride = sizeof(glm::vec2),
                           .debugName = "Buffer-GeometryTexCoords"})},
      normalBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.allocationStrategy = AllocationStrategy::Arena,
                           .bufferLifetime = BufferLifetime::Persistent,
                           .initialSize = NormalBufferInitialSize,
                           .itemStride = sizeof(glm::vec3),
                           .debugName = "Buffer-GeometryNormal"})},
      animationBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.allocationStrategy = AllocationStrategy::Arena,
                           .bufferLifetime = BufferLifetime::Persistent,
                           .initialSize = AnimationBufferInitialSize,
                           .itemStride = sizeof(glm::vec4),
                           .debugName = "Buffer-AnimationData"})} {
  aliasRegistry->setHandle(GlobalBufferAlias::Index, indexBuffer);
  aliasRegistry->setHandle(GlobalBufferAlias::Position, positionBuffer);
  aliasRegistry->setHandle(GlobalBufferAlias::Color, colorBuffer);
  aliasRegistry->setHandle(GlobalBufferAlias::TexCoord, texCoordBuffer);
  aliasRegistry->setHandle(GlobalBufferAlias::Normal, normalBuffer);
  aliasRegistry->setHandle(GlobalBufferAlias::Animation, animationBuffer);
}

auto GeometryBufferPack::getIndexBuffer() const -> const Handle<ManagedBuffer>& {
  return indexBuffer;
}

auto GeometryBufferPack::getPositionBuffer() const -> const Handle<ManagedBuffer>& {
  return positionBuffer;
}

auto GeometryBufferPack::getColorBuffer() const -> const Handle<ManagedBuffer>& {
  return colorBuffer;
}

auto GeometryBufferPack::getTexCoordBuffer() const -> const Handle<ManagedBuffer>& {
  return texCoordBuffer;
}

auto GeometryBufferPack::getNormalBuffer() const -> const Handle<ManagedBuffer>& {
  return normalBuffer;
}

auto GeometryBufferPack::getAnimationBuffer() const -> const Handle<ManagedBuffer>& {
  return animationBuffer;
}

auto GeometryBufferPack::checkSizes(const GeometryData& data) -> std::vector<ResizeRequest> {
  auto resizes = std::vector<ResizeRequest>{};

  auto check = [&](Handle<ManagedBuffer> handle, size_t size) {
    if (size == 0) {
      return;
    }
    if (auto maybe = bufferSystem->checkSize(handle, size)) {
      resizes.push_back(*maybe);
    }
  };

  check(indexBuffer, data.indexData->size());
  check(positionBuffer, data.positionData->size());
  check(colorBuffer, data.colorData ? data.colorData->size() : 0L);
  check(texCoordBuffer, data.texCoordData ? data.texCoordData->size() : 0L);
  check(normalBuffer, data.normalData ? data.normalData->size() : 0L);
  check(animationBuffer, data.animationData ? data.animationData->size() : 0L);

  return resizes;
}

auto GeometryBufferPack::allocate(Handle<ManagedBuffer> handle, const BufferRequest& bufferRequest)
    -> BufferRegion {
  return bufferSystem->allocate(handle, bufferRequest.size);
};

auto GeometryBufferPack::allocateIndexBuffer(const BufferRequest& bufferRequest) -> BufferRegion {
  return bufferSystem->allocate(indexBuffer, bufferRequest.size);
}

auto GeometryBufferPack::allocatePositionBuffer(const BufferRequest& bufferRequest)
    -> BufferRegion {
  return bufferSystem->allocate(positionBuffer, bufferRequest.size);
}

auto GeometryBufferPack::allocateColorBuffer(const BufferRequest& bufferRequest) -> BufferRegion {
  return bufferSystem->allocate(colorBuffer, bufferRequest.size);
}

auto GeometryBufferPack::allocateTexCoordBuffer(const BufferRequest& bufferRequest)
    -> BufferRegion {
  return bufferSystem->allocate(texCoordBuffer, bufferRequest.size);
}

auto GeometryBufferPack::allocateNormalBuffer(const BufferRequest& bufferRequest) -> BufferRegion {
  return bufferSystem->allocate(normalBuffer, bufferRequest.size);
}

auto GeometryBufferPack::allocateAnimationBuffer(const BufferRequest& bufferRequest)
    -> BufferRegion {
  return bufferSystem->allocate(animationBuffer, bufferRequest.size);
}

}
