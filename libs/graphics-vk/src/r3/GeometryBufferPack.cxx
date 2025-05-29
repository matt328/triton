#include "GeometryBufferPack.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"
#include "resources/allocators/IBufferAllocator.hpp"
#include "resources/allocators/LinearAllocator.hpp"

namespace tr {

GeometryBufferPack::GeometryBufferPack(const std::shared_ptr<BufferSystem>& bufferSystem)
    : indexBuffer{bufferSystem->registerBuffer(
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
  indexBufferAllocator = std::make_unique<LinearAllocator>(IndexBufferInitialSize);
  positionBufferAllocator = std::make_unique<LinearAllocator>(PositionBufferInitialSize);
  colorBufferAllocator = std::make_unique<LinearAllocator>(ColorBufferInitialSize);
  texCoordBufferAllocator = std::make_unique<LinearAllocator>(TexCoordBufferInitialSize);
  normalBufferAllocator = std::make_unique<LinearAllocator>(NormalBufferInitialSize);
  animationBufferAllocator = std::make_unique<LinearAllocator>(AnimationBufferInitialSize);
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

auto GeometryBufferPack::getIndexBufferAllocator() -> IBufferAllocator& {
  return *indexBufferAllocator;
}

auto GeometryBufferPack::getPositionBufferAllocator() -> IBufferAllocator& {
  return *positionBufferAllocator;
}

auto GeometryBufferPack::getColorBufferAllocator() -> IBufferAllocator& {
  return *colorBufferAllocator;
}

auto GeometryBufferPack::getTexCoordBufferAllocator() -> IBufferAllocator& {
  return *texCoordBufferAllocator;
}

auto GeometryBufferPack::getNormalBufferAllocator() -> IBufferAllocator& {
  return *normalBufferAllocator;
}

auto GeometryBufferPack::getAnimationBufferAllocator() -> IBufferAllocator& {
  return *animationBufferAllocator;
}

}
