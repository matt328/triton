#pragma once

#include "bk/Handle.hpp"
#include "dd/LogicalBufferHandle.hpp"
#include "dd/buffer-registry/BufferRegistry.hpp"
#include "img/ImageRequest.hpp"
#include "img/ManagedImage.hpp"
#include "vk/core/Swapchain.hpp"

namespace tr {

class Frame;

using LogicalImageHandle = size_t;

class IFrameManager {

public:
  IFrameManager() = default;
  virtual ~IFrameManager() = default;

  IFrameManager(const IFrameManager&) = delete;
  IFrameManager(IFrameManager&&) = delete;
  auto operator=(const IFrameManager&) -> IFrameManager& = delete;
  auto operator=(IFrameManager&&) -> IFrameManager& = delete;

  virtual auto acquireFrame() -> std::variant<Frame*, ImageAcquireResult> = 0;
  [[nodiscard]] virtual auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>& = 0;
  virtual auto registerPerFrameDrawImage(vk::Extent2D extent) -> LogicalImageHandle = 0;
  virtual auto registerPerFrameDepthImage(vk::Extent2D extent, vk::Format format)
      -> LogicalImageHandle = 0;
  virtual auto createPerFrameBuffer(const BufferUsageProfile& profile, size_t drawContextId)
      -> LogicalBufferHandle = 0;

  /// Creates an image for each frame, and returns a logical handle
  virtual auto registerImageRequest(const ImageRequest& request) -> Handle<ManagedImage> = 0;
};

}
