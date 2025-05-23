#pragma once

#include "IBufferAllocator.hpp"

namespace tr {

class AnimationAllocator : public IBufferAllocator {
public:
  explicit AnimationAllocator(Handle<ManagedBuffer> destinationBuffer);
  ~AnimationAllocator() override = default;

  AnimationAllocator(const AnimationAllocator&) = default;
  AnimationAllocator(AnimationAllocator&&) = delete;
  auto operator=(const AnimationAllocator&) -> AnimationAllocator& = default;
  auto operator=(AnimationAllocator&&) -> AnimationAllocator& = delete;

  auto allocate(const UploadRequestData& requestData) -> std::optional<MeshBufferRegion> override;
  auto reset() -> void override;

private:
  Handle<ManagedBuffer> positionBuffer;
  size_t insertOffset{};

  auto handleAnimationDataRequest(size_t resourceId,
                                  size_t newOffset,
                                  size_t newStagingCursor,
                                  const AnimationDataRequest& g) -> std::optional<MeshBufferRegion>;
};
}
