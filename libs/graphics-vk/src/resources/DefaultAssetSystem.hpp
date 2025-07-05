#pragma once

#include "api/fx/Events.hpp"
#include "as/StaticVertex.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "gfx/HandleMapperTypes.hpp"
#include "gfx/IAssetSystem.hpp"

namespace tr {

class IEventQueue;
class IAssetService;
class TrackerManager;
class BufferSystem;
class GeometryBufferPack;
class TransferSystem;
class GeometryAllocator;
class ImageManager;
class TextureArena;

constexpr uint32_t MaxBatchSize = 5;

class DefaultAssetSystem : public IAssetSystem {
public:
  explicit DefaultAssetSystem(std::shared_ptr<IEventQueue> newEventQueue,
                              std::shared_ptr<IAssetService> newAssetService,
                              std::shared_ptr<BufferSystem> newBufferSystem,
                              std::shared_ptr<GeometryBufferPack> newGeometryBufferPack,
                              std::shared_ptr<TransferSystem> newTransferSystem,
                              std::shared_ptr<GeometryAllocator> newGeometryAllocator,
                              std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
                              std::shared_ptr<TextureHandleMapper> newTextureHandleMapper,
                              std::shared_ptr<ImageManager> newImageManager,
                              std::shared_ptr<TextureArena> newTextureArena);
  ~DefaultAssetSystem() override;

  DefaultAssetSystem(const DefaultAssetSystem&) = delete;
  DefaultAssetSystem(DefaultAssetSystem&&) = delete;
  auto operator=(const DefaultAssetSystem&) -> DefaultAssetSystem& = delete;
  auto operator=(DefaultAssetSystem&&) -> DefaultAssetSystem& = delete;

  auto run() -> void override;
  auto requestStop() -> void override;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<BufferSystem> bufferSystem;
  std::shared_ptr<GeometryBufferPack> geometryBufferPack;
  std::shared_ptr<TransferSystem> transferSystem;
  std::shared_ptr<GeometryAllocator> geometryAllocator;
  std::shared_ptr<GeometryHandleMapper> geometryHandleMapper;
  std::shared_ptr<TextureHandleMapper> textureHandleMapper;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<TextureArena> textureArena;

  std::jthread thread;

  std::unordered_map<uint64_t, std::vector<const EventVariant*>> eventBatches;

  auto handleEndResourceBatch(uint64_t batchId) -> void;

  auto handleStaticModelRequest(const StaticModelRequest& smRequest,
                                UploadPlan& uploadPlan,
                                ImageUploadPlan& imageUploadPlan,
                                std::vector<StaticModelUploaded>& responses) -> void;

  auto handleStaticMeshRequest(const StaticMeshRequest& smRequest,
                               UploadPlan& uploadPlan,
                               std::vector<StaticModelUploaded>& responses) -> void;

  /// Eventually Update the TRM model formats to store data on disk in a deinterleaved format so
  /// this method is unnecessary, but just convert it here for now.
  auto deInterleave(const std::vector<as::StaticVertex>& vertices,
                    const std::vector<uint32_t>& indexData) -> std::unique_ptr<GeometryData>;
  auto fromImageData(const as::ImageData& imageData, uint64_t requestId)
      -> std::vector<ImageUploadData>;

  static auto getVkFormat(int bits, int component) -> vk::Format;
};

}
