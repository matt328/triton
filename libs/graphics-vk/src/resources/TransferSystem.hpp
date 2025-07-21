#pragma once

#include "ImageTransitionQueue.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "resources/TransferContext.hpp"
#include "resources/allocators/IBufferAllocator.hpp"
#include "resources/processors/StagingRequirements.hpp"
#include "gfx/HandleMapperTypes.hpp"

namespace tr {

class BufferSystem;
class CommandBufferManager;
class Device;
class ImageManager;
class ImageTransitionQueue;
class TextureArena;

namespace queue {
class Transfer;
class Graphics;
}

using BufferPair = std::vector<std::tuple<ManagedBuffer*, ManagedBuffer*>>;

struct DefragRequest {};

class TransferSystem {
public:
  explicit TransferSystem(std::shared_ptr<BufferSystem> newBufferSystem,
                          std::shared_ptr<Device> newDevice,
                          std::shared_ptr<queue::Transfer> newTransferQueue,
                          std::shared_ptr<queue::Graphics> newGraphicsQueue,
                          std::shared_ptr<ImageManager> newImageManager,
                          std::shared_ptr<ImageTransitionQueue> newImageQueue,
                          std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
                          std::shared_ptr<TextureArena> newTextureArena,
                          std::shared_ptr<TextureHandleMapper> newTextureHandleMapper,
                          const std::shared_ptr<CommandBufferManager>& commandBufferManager);
  ~TransferSystem() = default;

  TransferSystem(const TransferSystem&) = delete;
  TransferSystem(TransferSystem&&) = delete;
  auto operator=(const TransferSystem&) -> TransferSystem& = delete;
  auto operator=(TransferSystem&&) -> TransferSystem& = delete;

  auto upload2(const UploadSubBatch& subBatch) -> std::vector<SubBatchResult>;
  auto defragment(const DefragRequest& defrag) -> void;

  auto getTransferContext() -> TransferContext&;
  auto getGeometryStagingBufferSize() -> size_t;
  auto getImageStagingBufferSize() -> size_t;
  auto copyBuffers(const BufferPair& bufferPair) -> void;

private:
  std::shared_ptr<BufferSystem> bufferSystem;
  std::shared_ptr<Device> device;
  std::shared_ptr<queue::Transfer> transferQueue;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<ImageTransitionQueue> imageQueue;
  std::shared_ptr<GeometryHandleMapper> geometryHandleMapper;
  std::shared_ptr<TextureArena> textureArena;
  std::shared_ptr<TextureHandleMapper> textureHandleMapper;

  std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
  std::unique_ptr<vk::raii::Fence> fence = nullptr;
  std::vector<ImageTransitionInfo> transitionBatch{};

  TransferContext transferContext;

  /// DstBuffer to BufferCopy2's into said buffer
  using BufferCopyMap = std::unordered_map<Handle<ManagedBuffer>, std::vector<vk::BufferCopy2>>;
  using ImageCopyMap = std::unordered_map<Handle<ManagedImage>, std::vector<vk::BufferImageCopy2>>;

  auto checkSizes(const UploadSubBatch& uploadSubBatch)
      -> std::tuple<std::vector<ResizeRequest>, std::vector<ResizeRequest>>;

  auto processResizes(const std::vector<ResizeRequest>& resizeRequestList) -> void;

  auto prepareStagingData(const UploadSubBatch& subBatch)
      -> std::tuple<BufferCopyMap, ImageCopyMap>;

  auto prepareBufferStagingData(const UploadPlan& bufferPlan) -> BufferCopyMap;
  auto prepareImageStagingData(const ImageUploadPlan& imagePlan) -> ImageCopyMap;

  auto recordBufferUploads(const BufferCopyMap& bufferCopies) -> void;
  auto recordImageUploads(const ImageCopyMap& imageCopies) -> void;

  auto submitAndWait() -> void;
};

}
