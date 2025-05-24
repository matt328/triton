#include "GeometryBufferPack.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"

namespace tr {

GeometryBufferPack::GeometryBufferPack(const std::shared_ptr<BufferSystem>& bufferSystem)
    : indexBuffer{bufferSystem->registerBuffer(
          BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                           .itemStride = sizeof(uint32_t),
                           .debugName = "GeometryIndex"})},
      positionBuffer{
          bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                                                        .itemStride = sizeof(glm::vec3),
                                                        .debugName = "GeometryPosition"})},
      colorBuffer{
          bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                                                        .itemStride = sizeof(glm::vec4),
                                                        .debugName = "GeometryColors"})},
      texCoordBuffer{
          bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                                                        .itemStride = sizeof(glm::vec2),
                                                        .debugName = "GeometryTexCoords"})},
      normalBuffer{
          bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                                                        .itemStride = sizeof(glm::vec3),
                                                        .debugName = "GeometryNormal"})},
      animationBuffer{
          bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                                                        .itemStride = sizeof(glm::vec4),
                                                        .debugName = "AnimationData"})} {
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

}
