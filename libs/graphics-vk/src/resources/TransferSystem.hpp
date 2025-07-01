#pragma once

#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "resources/TransferContext.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

class BufferSystem;
class CommandBufferManager;
class Device;

namespace queue {
class Transfer;
}

struct ResizeRequest {};
struct DefragRequest {};

class TransferSystem {
public:
  explicit TransferSystem(std::shared_ptr<BufferSystem> newBufferSystem,
                          std::shared_ptr<Device> newDevice,
                          std::shared_ptr<queue::Transfer> newTransferQueue,
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

  std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
  std::unique_ptr<vk::raii::Fence> fence = nullptr;

  TransferContext transferContext;

  auto checkSizes(const UploadPlan& uploadPlan) -> std::vector<ResizeRequest>;
  auto checkImageSizes(const ImageUploadPlan& imagePlan) -> std::vector<ResizeRequest>;

  auto processResizes(const std::vector<ResizeRequest>& resizeRequestList,
                      const std::vector<ResizeRequest>& imageResizeRequestList) -> void;

  /// DstBuffer to BufferCopy2's into said buffer
  using BufferCopyMap = std::unordered_map<Handle<ManagedBuffer>, std::vector<vk::BufferCopy2>>;

  auto prepareBufferStagingData(const UploadPlan& bufferPlan) -> BufferCopyMap;
  auto prepareImageStagingData(const ImageUploadPlan& imagePlan) -> void;

  auto recordBufferUploads(const BufferCopyMap& bufferCopies) -> void;
  auto recordImageUploads(const ImageUploadPlan& imagePlan) -> void;

  auto submitAndWait() -> void;
};

}
