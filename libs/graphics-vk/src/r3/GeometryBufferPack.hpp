#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {

class BufferSystem;
class IBufferAllocator;

constexpr size_t IndexBufferInitialSize = 1024000;
constexpr size_t PositionBufferInitialSize = 2671552;
constexpr size_t ColorBufferInitialSize = 1024000;
constexpr size_t TexCoordBufferInitialSize = 1024000;
constexpr size_t NormalBufferInitialSize = 1024000;
constexpr size_t AnimationBufferInitialSize = 1024000;

class GeometryBufferPack {
public:
  explicit GeometryBufferPack(const std::shared_ptr<BufferSystem>& bufferSystem);
  ~GeometryBufferPack() = default;

  GeometryBufferPack(const GeometryBufferPack&) = delete;
  GeometryBufferPack(GeometryBufferPack&&) = delete;
  auto operator=(const GeometryBufferPack&) -> GeometryBufferPack& = delete;
  auto operator=(GeometryBufferPack&&) -> GeometryBufferPack& = delete;

  [[nodiscard]] auto getIndexBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getPositionBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getColorBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getTexCoordBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getNormalBuffer() const -> const Handle<ManagedBuffer>&;
  [[nodiscard]] auto getAnimationBuffer() const -> const Handle<ManagedBuffer>&;

  auto getIndexBufferAllocator() -> IBufferAllocator&;
  auto getPositionBufferAllocator() -> IBufferAllocator&;
  auto getColorBufferAllocator() -> IBufferAllocator&;
  auto getTexCoordBufferAllocator() -> IBufferAllocator&;
  auto getNormalBufferAllocator() -> IBufferAllocator&;
  auto getAnimationBufferAllocator() -> IBufferAllocator&;

private:
  Handle<ManagedBuffer> indexBuffer;
  Handle<ManagedBuffer> positionBuffer;
  Handle<ManagedBuffer> colorBuffer;
  Handle<ManagedBuffer> texCoordBuffer;
  Handle<ManagedBuffer> normalBuffer;
  Handle<ManagedBuffer> animationBuffer;

  std::unique_ptr<IBufferAllocator> indexBufferAllocator;
  std::unique_ptr<IBufferAllocator> positionBufferAllocator;
  std::unique_ptr<IBufferAllocator> colorBufferAllocator;
  std::unique_ptr<IBufferAllocator> texCoordBufferAllocator;
  std::unique_ptr<IBufferAllocator> normalBufferAllocator;
  std::unique_ptr<IBufferAllocator> animationBufferAllocator;
};

}
