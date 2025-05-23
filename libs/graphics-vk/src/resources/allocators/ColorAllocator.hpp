#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class ColorAllocator : public IBufferAllocator {
public:
  explicit ColorAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~ColorAllocator() override = default;

  ColorAllocator(const ColorAllocator&) = default;
  ColorAllocator(ColorAllocator&&) = delete;
  auto operator=(const ColorAllocator&) -> ColorAllocator& = default;
  auto operator=(ColorAllocator&&) -> ColorAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> positionBuffer;
  size_t insertOffset{};

  auto handleColorDataRequest(size_t resourceId,
                              size_t newOffset,
                              size_t newStagingCursor,
                              const ColorDataRequest& g) -> std::optional<MeshBufferRegion>;
};
}
