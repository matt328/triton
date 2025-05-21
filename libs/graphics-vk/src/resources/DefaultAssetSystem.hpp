#pragma once

#include "as/StaticVertex.hpp"
#include "bk/HandleGenerator.hpp"
#include "gfx/IAssetSystem.hpp"
#include "resources/AssetTypes.hpp"

namespace tr {

class IEventQueue;
class IAssetService;
class TrackerManager;

class DefaultAssetSystem : public IAssetSystem {
public:
  explicit DefaultAssetSystem(std::shared_ptr<IEventQueue> newEventQueue,
                              std::shared_ptr<IAssetService> newAssetService);
  ~DefaultAssetSystem() override;

  DefaultAssetSystem(const DefaultAssetSystem&) = delete;
  DefaultAssetSystem(DefaultAssetSystem&&) = delete;
  auto operator=(const DefaultAssetSystem&) -> DefaultAssetSystem& = delete;
  auto operator=(DefaultAssetSystem&&) -> DefaultAssetSystem& = delete;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IAssetService> assetService;
  std::shared_ptr<TrackerManager> trackerManager;

  std::thread workerThread;
  std::atomic_bool running{true};

  moodycamel::ReaderWriterQueue<AssetTask> taskQueue;

  HandleGenerator<GeometryData> geometryHandleGenerator;
  std::unordered_map<Handle<GeometryData>, std::unique_ptr<GeometryData>> geometryDataMap;

  auto assetWorkerThreadFn() -> void;

  auto handleStaticModelRequest(const StaticModelRequest& smRequest) -> void;
  auto handleStaticModelTask(const StaticModelTask& smTask) -> void;

  auto handleGeometryUploaded(const UploadGeometryResponse& uploaded) -> void;
  auto handleImageUploaded(const UploadImageResponse& uploaded) -> void;

  static auto fromRequest(const AssetRequest& request) -> AssetTask;

  /// Eventually Update the TRM model formats to store data on disk in a deinterleaved format so
  /// this method is unnecessary, but just convert it here for now.
  static auto deInterleave(const std::vector<as::StaticVertex>& vertices,
                           const std::vector<uint32_t>& indexData) -> std::unique_ptr<GeometryData>;
};

}
