#include "DefaultAssetSystem.hpp"
#include "api/fx/IAssetService.hpp"
#include "api/fx/IEventQueue.hpp"
#include "as/Model.hpp"
#include "buffers/BufferSystem.hpp"
#include "buffers/UploadPlan.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "resources/ByteConverters.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"

namespace tr {

DefaultAssetSystem::DefaultAssetSystem(
    std::shared_ptr<IEventQueue> newEventQueue,
    std::shared_ptr<IAssetService> newAssetService,
    std::shared_ptr<BufferSystem> newBufferSystem,
    std::shared_ptr<GeometryBufferPack> newGeometryBufferPack,
    std::shared_ptr<TransferSystem> newTransferSystem,
    std::shared_ptr<GeometryAllocator> newGeometryAllocator,
    std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper)
    : eventQueue{std::move(newEventQueue)},
      assetService{std::move(newAssetService)},
      bufferSystem{std::move(newBufferSystem)},
      geometryBufferPack{std::move(newGeometryBufferPack)},
      transferSystem{std::move(newTransferSystem)},
      geometryAllocator{std::move(newGeometryAllocator)},
      geometryHandleMapper{std::move(newGeometryHandleMapper)} {
  Log.trace("Constructing DefaultAssetSystem");
}

DefaultAssetSystem::~DefaultAssetSystem() {
  Log.trace("Destroying DefaultAssetSystem");
}

auto DefaultAssetSystem::run() -> void {
  ZoneScopedN("DefaultAssetSystem::run");
  Log.trace("DefaultAssetSystem::run()");

  thread = std::jthread([&](std::stop_token token) mutable {
    pthread_setname_np(pthread_self(), "AssetSystem");
    Log.trace("Started AssetSystemThread");
    // Create all subscriptions on the thread
    eventQueue->subscribe<BeginResourceBatch>(
        [this](const BeginResourceBatch& batch) {
          eventBatches[batch.batchId] = std::vector<const EventVariant*>{};
        },
        "test_group");

    eventQueue->subscribe<StaticModelRequest>(
        [this](const StaticModelRequest& smRequest, const EventVariant& eventVariant) {
          eventBatches[smRequest.batchId].push_back(&eventVariant);
        },
        "test_group");

    eventQueue->subscribe<DynamicModelRequest>(
        [this](const DynamicModelRequest& dmRequest, const EventVariant& eventVariant) {
          eventBatches[dmRequest.batchId].push_back(&eventVariant);
        },
        "test_group");

    eventQueue->subscribe<EndResourceBatch>(
        [this](const EndResourceBatch& batch) { handleEndResourceBatch(batch.batchId); },
        "test_group");

    while (!token.stop_requested()) {
      eventQueue->dispatchPending();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    Log.trace("AssetSystem thread shutting down");
  });
}

auto DefaultAssetSystem::requestStop() -> void {
  thread.request_stop();
}

auto DefaultAssetSystem::handleEndResourceBatch(uint64_t batchId) -> void {
  ZoneScoped;
  auto uploadPlan = UploadPlan{.stagingBuffer = transferSystem->getTransferContext().stagingBuffer};
  std::vector<StaticModelUploaded> responses{};

  std::vector<as::Model> loadedModels{};

  for (auto& eventVariant : eventBatches[batchId]) {
    auto visitor = [&](auto&& arg) -> void {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, StaticModelRequest>) {
        handleStaticModelRequest(arg, uploadPlan, responses, loadedModels);
      }
      if constexpr (std::is_same_v<T, DynamicModelRequest>) {
        Log.trace("Handling Dynamic Model Request ID: {}", arg.requestId);
      }
    };
    std::visit(visitor, *eventVariant);
  }

  transferSystem->upload(uploadPlan);

  loadedModels.clear();

  for (const auto& response : responses) {
    eventQueue->emit(response);
  }
}

auto DefaultAssetSystem::handleStaticModelRequest(const StaticModelRequest& smRequest,
                                                  UploadPlan& uploadPlan,
                                                  std::vector<StaticModelUploaded>& responses,
                                                  std::vector<as::Model>& loadedModels) -> void {
  ZoneScoped;
  Log.trace("Handling Static Model Request ID: {}", smRequest.requestId);
  loadedModels.push_back(assetService->loadModel(smRequest.modelFilename));
  const auto& model = loadedModels.back();

  const auto geometryData = deInterleave(*model.staticVertices, model.indices);

  const auto [regionHandle, uploads] =
      geometryAllocator->allocate(*geometryData, transferSystem->getTransferContext());

  responses.push_back(StaticModelUploaded{
      .batchId = smRequest.batchId,
      .requestId = smRequest.requestId,
      .entityName = smRequest.entityName,
      .geometryHandle = geometryHandleMapper->toPublic(regionHandle),
  });

  uploadPlan.uploads.insert(uploadPlan.uploads.end(), uploads.begin(), uploads.end());
  const auto imageUploadData = fromImageData(model.imageData);
  uploadPlan.uploads.insert(uploadPlan.uploads.end(),
                            imageUploadData.begin(),
                            imageUploadData.end());
}

auto DefaultAssetSystem::deInterleave(const std::vector<as::StaticVertex>& vertices,
                                      const std::vector<uint32_t>& indexData)
    -> std::unique_ptr<GeometryData> {
  auto positions = std::make_shared<std::vector<GpuVertexPositionData>>();
  auto texCoords = std::make_shared<std::vector<GpuVertexTexCoordData>>();
  auto indices = std::make_shared<std::vector<GpuIndexData>>();

  positions->reserve(vertices.size() * sizeof(GpuVertexPositionData));
  texCoords->reserve(vertices.size() * sizeof(GpuVertexTexCoordData));
  indices->reserve(indexData.size() * sizeof(GpuIndexData));

  for (const auto& vertex : vertices) {
    positions->emplace_back(vertex.position);
    texCoords->emplace_back(vertex.texCoord);
  }

  for (auto index : indexData) {
    indices->emplace_back(index);
  }

  auto indicesBytes = toByteVector(indices);
  auto texCoordBytes = toByteVector(texCoords);
  auto positionBytes = toByteVector(positions);

  return std::make_unique<GeometryData>(GeometryData{.indexData = indicesBytes,
                                                     .positionData = positionBytes,
                                                     .colorData = nullptr,
                                                     .texCoordData = texCoordBytes,
                                                     .normalData = nullptr,
                                                     .animationData = nullptr});
}

auto DefaultAssetSystem::fromImageData([[maybe_unused]] const as::ImageData& imageData)
    -> std::vector<UploadData> {
  return {};
}

}
