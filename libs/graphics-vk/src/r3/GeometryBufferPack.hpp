#pragma once

#include "api/gfx/GeometryData.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "resources/TransferSystem.hpp"

namespace tr {

class BufferSystem;
class ResourceAliasRegistry;
class IBufferAllocator;

constexpr size_t IndexBufferInitialSize = 1024000;
constexpr size_t PositionBufferInitialSize = 2671552;
constexpr size_t ColorBufferInitialSize = 1024000;
constexpr size_t TexCoordBufferInitialSize = 1024000;
constexpr size_t NormalBufferInitialSize = 1024000;
constexpr size_t AnimationBufferInitialSize = 1024000;

class GeometryBufferPack {
public:
  GeometryBufferPack(std::shared_ptr<BufferSystem> newBufferSystem,
                     const std::shared_ptr<ResourceAliasRegistry>& aliasRegistry);
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

  /// Checks to see if any buffers need resized to hold this data. Returns a vector of
  /// ResizeRequests containing each buffer that needs resized, and the buffer's new size.
  auto checkSizes(const GeometryData& data) -> std::vector<ResizeRequest>;

  auto allocate(Handle<ManagedBuffer> handle, const BufferRequest& bufferRequest) -> BufferRegion;
  auto allocateIndexBuffer(const BufferRequest& bufferRequest) -> BufferRegion;
  auto allocatePositionBuffer(const BufferRequest& bufferRequest) -> BufferRegion;
  auto allocateColorBuffer(const BufferRequest& bufferRequest) -> BufferRegion;
  auto allocateTexCoordBuffer(const BufferRequest& bufferRequest) -> BufferRegion;
  auto allocateNormalBuffer(const BufferRequest& bufferRequest) -> BufferRegion;
  auto allocateAnimationBuffer(const BufferRequest& bufferRequest) -> BufferRegion;

private:
  std::shared_ptr<BufferSystem> bufferSystem;

  Handle<ManagedBuffer> indexBuffer;
  Handle<ManagedBuffer> positionBuffer;
  Handle<ManagedBuffer> colorBuffer;
  Handle<ManagedBuffer> texCoordBuffer;
  Handle<ManagedBuffer> normalBuffer;
  Handle<ManagedBuffer> animationBuffer;
};

}
