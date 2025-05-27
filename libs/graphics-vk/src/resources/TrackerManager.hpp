#pragma once

#include "bk/Handle.hpp"

namespace tr {

struct TempModelTag {};

struct ModelLoadTracker {
  int remainingTasks = 0;
  Handle<TempModelTag> handle;
  std::function<void()> onComplete;
};

using TrackerVariant = std::variant<ModelLoadTracker>;
using TrackerPtr = std::shared_ptr<TrackerVariant>;
using EventId = uint64_t;

class TrackerManager {
public:
  TrackerManager() = default;
  ~TrackerManager() = default;

  TrackerManager(const TrackerManager&) = delete;
  TrackerManager(TrackerManager&&) = delete;
  auto operator=(const TrackerManager&) -> TrackerManager& = delete;
  auto operator=(TrackerManager&&) -> TrackerManager& = delete;

  auto add(EventId id, TrackerPtr tracker) {
    std::scoped_lock lock(mutex);
    trackers.emplace(id, std::move(tracker));
  }

  template <typename TrackerType, typename Func>
  void with(EventId id, Func&& func) {
    std::scoped_lock lock(mutex);
    auto it = trackers.find(id);
    if (it == trackers.end()) {
      return;
    }
    if (auto* ptr = std::get_if<TrackerType>(&*it->second)) {
      func(*ptr);
    }
  }

  auto remove(size_t id) {
    std::scoped_lock lock(mutex);
    trackers.erase(id);
  }

private:
  std::unordered_map<size_t, TrackerPtr> trackers;
  std::mutex mutex;
};

}

/*
  use this like this:

  auto DefaultAssetSystem::handleStaticModelTask(const StaticModelTask& smTask) -> void {
  Log.trace("Handling StaticModelTask, loading model");
  auto model = assetService->loadModel(smTask.filename);
  auto geometryData = deInterleave(model.staticVertices.value(), model.indices);
  auto imageData = std::make_unique<as::ImageData>(model.imageData);

  auto tracker = std::make_shared<ModelLoadTracker>(ModelLoadTracker{.remainingTasks = 2});

  tracker->onComplete = [this, task = smTask, tracker]() {
    Log.trace("Tracker::onComplete id={}", task.id);
    auto event = StaticModelResponse{.batchId = task.batchId,
                                     .requestId = task.id,
                                     .entityName = task.entityName};
    eventQueue->emit(event, "test_group");
  };

  auto variantTracker = std::make_shared<TrackerVariant>(std::move(*tracker));
  trackerManager->add(smTask.id, variantTracker);

  Log.trace("Emitting UploadGeometryRequest id={}", smTask.id);
  eventQueue->emit(UploadGeometryRequest{.batchId = smTask.batchId,
                                         .requestId = smTask.id,
                                         .data = std::move(geometryData)});
  eventQueue->emit(UploadImageRequest{.batchId = smTask.batchId,
                                      .requestId = smTask.id,
                                      .data = std::move(imageData)});
}
*/
