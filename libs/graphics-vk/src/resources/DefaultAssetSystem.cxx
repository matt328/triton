#include "DefaultAssetSystem.hpp"
#include "api/fx/IAssetService.hpp"
#include "api/fx/IEventQueue.hpp"
#include "as/Model.hpp"
#include "resources/AssetTypes.hpp"
#include "resources/TrackerManager.hpp"

namespace tr {

DefaultAssetSystem::DefaultAssetSystem(std::shared_ptr<IEventQueue> newEventQueue,
                                       std::shared_ptr<IAssetService> newAssetService)
    : eventQueue{std::move(newEventQueue)}, assetService{std::move(newAssetService)} {
  Log.trace("Constructing DefaultAssetSystem");

  trackerManager = std::make_shared<TrackerManager>();

  eventQueue->subscribe<StaticModelRequest>(
      [this](const StaticModelRequest& smRequest) { handleStaticModelRequest(smRequest); });

  eventQueue->subscribe<UploadGeometryResponse>(
      [this](const UploadGeometryResponse& uploaded) { handleGeometryUploaded(uploaded); });

  workerThread = std::thread([this] { assetWorkerThreadFn(); });
}

DefaultAssetSystem::~DefaultAssetSystem() {
  running = false;
  if (workerThread.joinable()) {
    workerThread.join();
  }
}

auto DefaultAssetSystem::handleStaticModelRequest(const StaticModelRequest& smRequest) -> void {
  auto task = fromRequest(smRequest);
  while (!taskQueue.try_enqueue(task)) {
    std::this_thread::yield();
  }
}

auto DefaultAssetSystem::assetWorkerThreadFn() -> void {
  while (running) {
    AssetTask task;
    if (!taskQueue.try_dequeue(task)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    std::visit(
        [this](auto&& assetTask) {
          using T = std::decay_t<decltype(assetTask)>;
          if constexpr (std::is_same_v<T, StaticModelTask>) {
            handleStaticModelTask(assetTask);
          }
        },
        task);
  }
}

auto DefaultAssetSystem::handleStaticModelTask(const StaticModelTask& smTask) -> void {
  auto model = assetService->loadModel(smTask.filename);
  auto geometryData = deInterleave(model.staticVertices.value(), model.indices);
  auto imageData = model.imageData;

  auto tracker = std::make_shared<ModelLoadTracker>(ModelLoadTracker{.remainingTasks = 2});

  tracker->onComplete = [this, id = smTask.id, tracker]() {
    auto event = StaticModelResponse{.requestId = id};
    eventQueue->emit(event);
  };

  auto variantTracker = std::make_shared<TrackerVariant>(std::move(*tracker));
  trackerManager->add(smTask.id, variantTracker);

  eventQueue->emit(UploadGeometryRequest{.requestId = smTask.id, .data = std::move(geometryData)});
  // eventQueue->emit(UploadImageData{data = std::move(imageData)});
}

auto DefaultAssetSystem::handleGeometryUploaded(const UploadGeometryResponse& uploaded) -> void {
  trackerManager->with<ModelLoadTracker>(uploaded.requestId,
                                         [this, uploaded](ModelLoadTracker& tracker) {
                                           // tracker.handle = uploaded.geometryHandle;
                                           if (--tracker.remainingTasks == 0) {
                                             tracker.onComplete();
                                             trackerManager->remove(uploaded.requestId);
                                           }
                                         });
}

auto DefaultAssetSystem::deInterleave(const std::vector<as::StaticVertex>& vertices,
                                      const std::vector<uint32_t>& indexData)
    -> std::unique_ptr<GeometryData> {
  auto positions = std::vector<GpuVertexPositionData>{};
  auto texCoords = std::vector<GpuVertexTexCoordData>{};
  positions.resize(vertices.size());
  texCoords.resize(vertices.size());
  for (const auto& vertex : vertices) {
    positions.push_back({vertex.position});
    texCoords.push_back({vertex.texCoord});
  }
  auto indices = std::vector<GpuIndexData>{};
  indices.resize(indexData.size());
  for (const auto& index : indexData) {
    indices.push_back({index});
  }
  return std::make_unique<GeometryData>(
      GeometryData{.indexData = indices, .positionData = positions, .texCoordData = texCoords});
}

auto DefaultAssetSystem::fromRequest(const AssetRequest& request) -> AssetTask {
  return std::visit(
      [](auto&& req) {
        using T = std::decay_t<decltype(req)>;
        if constexpr (std::is_same_v<T, StaticModelRequest>) {
          return StaticModelTask{.filename = req.modelFilename};
        } else {
          static_assert(always_false<T>, "Unhandled AssetRequestType");
        }
      },
      request);
}

}
