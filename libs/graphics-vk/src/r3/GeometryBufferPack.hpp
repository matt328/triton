#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {

class BufferSystem;

class GeometryBufferPack {
public:
  explicit GeometryBufferPack(const std::shared_ptr<BufferSystem>& bufferSystem);
  ~GeometryBufferPack() = default;

  GeometryBufferPack(const GeometryBufferPack&) = default;
  GeometryBufferPack(GeometryBufferPack&&) = delete;
  auto operator=(const GeometryBufferPack&) -> GeometryBufferPack& = default;
  auto operator=(GeometryBufferPack&&) -> GeometryBufferPack& = delete;

  [[nodiscard]] auto getIndexBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getPositionBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getColorBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getTexCoordBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getNormalBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getAnimationBuffer() const -> const Handle<ManagedBuffer>&;

private:
  Handle<ManagedBuffer> indexBuffer;
  Handle<ManagedBuffer> positionBuffer;
  Handle<ManagedBuffer> colorBuffer;
  Handle<ManagedBuffer> texCoordBuffer;
  Handle<ManagedBuffer> normalBuffer;
  Handle<ManagedBuffer> animationBuffer;
};

}
