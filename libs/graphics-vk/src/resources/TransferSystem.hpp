#pragma once

#include "ImageTransitionQueue.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "resources/TransferContext.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

class BufferSystem;
class CommandBufferManager;
class Device;
class ImageManager;
class ImageTransitionQueue;

namespace queue {
class Transfer;
class Graphics;
}

struct ResizeRequest {};
struct DefragRequest {};

class TransferSystem {
public:
  explicit TransferSystem(std::shared_ptr<BufferSystem> newBufferSystem,
                          std::shared_ptr<Device> newDevice,
                          std::shared_ptr<queue::Transfer> newTransferQueue,
                          std::shared_ptr<queue::Graphics> newGraphicsQueue,
                          std::shared_ptr<ImageManager> newImageManager,
                          std::shared_ptr<ImageTransitionQueue> newImageQueue,
                          const std::shared_ptr<CommandBufferManager>& commandBufferManager);
  ~TransferSystem() = default;

  TransferSystem(const TransferSystem&) = delete;
  TransferSystem(TransferSystem&&) = delete;
  auto operator=(const TransferSystem&) -> TransferSystem& = delete;
  auto operator=(TransferSystem&&) -> TransferSystem& = delete;

  auto upload(UploadPlan& bufferPlan, ImageUploadPlan& imagePlan) -> void;
  auto enqueueResize(const ResizeRequest& resize) -> void;
  auto defragment(const DefragRequest& defrag) -> void;

  auto getTransferContext() -> TransferContext&;

private:
  std::shared_ptr<BufferSystem> bufferSystem;
  std::shared_ptr<Device> device;
  std::shared_ptr<queue::Transfer> transferQueue;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<ImageTransitionQueue> imageQueue;

  std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
  std::unique_ptr<vk::raii::Fence> fence = nullptr;
  std::vector<ImageTransitionInfo> transitionBatch{};

  TransferContext transferContext;

  auto checkSizes(const UploadPlan& uploadPlan) -> std::vector<ResizeRequest>;
  auto checkImageSizes(const ImageUploadPlan& imagePlan) -> std::vector<ResizeRequest>;

  auto processResizes(const std::vector<ResizeRequest>& resizeRequestList,
                      const std::vector<ResizeRequest>& imageResizeRequestList) -> void;

  /// DstBuffer to BufferCopy2's into said buffer
  using BufferCopyMap = std::unordered_map<Handle<ManagedBuffer>, std::vector<vk::BufferCopy2>>;
  using ImageCopyMap = std::unordered_map<Handle<ManagedImage>, std::vector<vk::BufferImageCopy2>>;

  auto prepareBufferStagingData(const UploadPlan& bufferPlan) -> BufferCopyMap;
  auto prepareImageStagingData(const ImageUploadPlan& imagePlan) -> ImageCopyMap;

  auto recordBufferUploads(const BufferCopyMap& bufferCopies) -> void;
  auto recordImageUploads(const ImageCopyMap& imageCopies) -> void;

  auto submitAndWait() -> void;
};

}
