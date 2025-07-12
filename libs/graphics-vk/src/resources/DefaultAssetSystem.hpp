#pragma once

#include "api/fx/Events.hpp"
#include "api/fx/IEventQueue.hpp"
#include "as/StaticVertex.hpp"
#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "gfx/HandleMapperTypes.hpp"
#include "gfx/IAssetSystem.hpp"
#include "resources/InFlightUpload.hpp"

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

struct EmitEventVisitor {
  std::shared_ptr<IEventQueue> eventQueue;

  template <typename T>
  void operator()(const T& event) const {
    eventQueue->emit(event);
  }
};

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

  using RequestVariant = std::variant<std::shared_ptr<StaticMeshRequest>,
                                      std::shared_ptr<StaticModelRequest>,
                                      std::shared_ptr<DynamicModelRequest>>;

  std::unordered_map<uint64_t, std::vector<RequestVariant>> eventBatches;

  InFlightUploadMap inFlightUploads;

  auto processesBatchedResources(uint64_t batchId) -> void;

  auto processStaticModelRequest(const std::shared_ptr<StaticModelRequest>& smRequest,
                                 UploadPlan& uploadPlan,
                                 ImageUploadPlan& imageUploadPlan) -> void;

  auto processStaticMeshRequest(const std::shared_ptr<StaticMeshRequest>& smRequest,
                                UploadPlan& uploadPlan) -> void;

  auto collectUploads(uint64_t batchId) -> std::tuple<UploadPlan, ImageUploadPlan>;

  auto finalizeResponses(UploadPlan& uploadPlan, ImageUploadPlan& imageUploadPlan) -> void;
};

}
