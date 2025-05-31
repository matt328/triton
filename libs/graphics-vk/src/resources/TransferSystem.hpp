#pragma once

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

  auto upload(UploadPlan& uploadPlan) -> void;
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
  auto processResizes(const std::vector<ResizeRequest>& resizeRequestList) -> void;
};

}
