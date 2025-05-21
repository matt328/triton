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

  eventQueue->subscribe<StaticModelResponse>([](const StaticModelResponse& response) {
    Log.trace("StaticModelResponse id={}", response.requestId);
  });
}

DefaultAssetSystem::~DefaultAssetSystem() {
  Log.trace("Destroying DefaultAssetSystem");
  running = false;
  if (workerThread.joinable()) {
    workerThread.join();
  }
}

auto DefaultAssetSystem::assetWorkerThreadFn() -> void {
  Log.trace("Starting AssetWorker Thread");
  pthread_setname_np(pthread_self(), "AssetWorker");
  while (running) {
    AssetTask task;
    if (!taskQueue.try_dequeue(task)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    Log.trace("Starting Task");
    std::visit(
        [this](auto&& assetTask) {
          using T = std::decay_t<decltype(assetTask)>;
          if constexpr (std::is_same_v<T, StaticModelTask>) {
            handleStaticModelTask(assetTask);
          }
        },
        task);
  }
  Log.trace("AssetWorker Thread Finished");
}

auto DefaultAssetSystem::handleStaticModelRequest(const StaticModelRequest& smRequest) -> void {
  Log.trace("Handling Static Model Request ID: {}", smRequest.requestId);
  auto task = fromRequest(smRequest);
  while (!taskQueue.try_enqueue(task)) {
    std::this_thread::yield();
  }
}

auto DefaultAssetSystem::handleStaticModelTask(const StaticModelTask& smTask) -> void {
  Log.trace("Handling StaticModelTask");
  auto model = assetService->loadModel(smTask.filename);
  auto geometryData = deInterleave(model.staticVertices.value(), model.indices);
  auto imageData = model.imageData;

  auto tracker = std::make_shared<ModelLoadTracker>(ModelLoadTracker{.remainingTasks = 1});

  tracker->onComplete = [this, id = smTask.id, tracker]() {
    Log.trace("Tracker::onComplete id={}", id);
    auto event = StaticModelResponse{.requestId = id};
    eventQueue->emit(event);
  };

  auto variantTracker = std::make_shared<TrackerVariant>(std::move(*tracker));
  trackerManager->add(smTask.id, variantTracker);

  Log.trace("Emitting UploadGeometryRequest id={}", smTask.id);
  eventQueue->emit(UploadGeometryRequest{.requestId = smTask.id, .data = std::move(geometryData)});
  // eventQueue->emit(UploadImageData{data = std::move(imageData)});
}

auto DefaultAssetSystem::handleGeometryUploaded(const UploadGeometryResponse& uploaded) -> void {
  Log.trace("Handling UploadGeometryResponse Id={}", uploaded.requestId);
  bool shouldRemove = false;
  trackerManager->with<ModelLoadTracker>(uploaded.requestId,
                                         [&shouldRemove](ModelLoadTracker& tracker) {
                                           // tracker.handle = uploaded.geometryHandle;
                                           if (--tracker.remainingTasks == 0) {
                                             tracker.onComplete();
                                             shouldRemove = true;
                                           }
                                         });
  if (shouldRemove) {
    trackerManager->remove(uploaded.requestId);
  }
  Log.trace("Finished Handling UploadGeometryResponse id={}", uploaded.requestId);
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
          return StaticModelTask{.id = req.requestId, .filename = req.modelFilename};
        } else {
          static_assert(always_false<T>, "Unhandled AssetRequestType");
        }
      },
      request);
}

}
