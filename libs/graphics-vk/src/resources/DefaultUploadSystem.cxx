#include "DefaultUploadSystem.hpp"
#include "api/fx/IEventQueue.hpp"
#include "buffers/DeviceBufferSystem.hpp"
#include "resources/allocators/GeometryDispatcher.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

DefaultUploadSystem::DefaultUploadSystem(std::shared_ptr<IEventQueue> newEventQueue,
                                         std::shared_ptr<DeviceBufferSystem> newDeviceBufferSystem)
    : eventQueue{std::move(newEventQueue)}, bufferSystem{std::move(newDeviceBufferSystem)} {
  Log.trace("Constructing DefaultUploadSystem");

  eventQueue->subscribe<UploadGeometryRequest>(
      [this](const UploadGeometryRequest& event) { handleUploadGeometry(event); });

  eventQueue->subscribe<UploadImageRequest>(
      [this](const UploadImageRequest& event) { handleUploadImage(event); });
}

auto DefaultUploadSystem::handleUploadGeometry([[maybe_unused]] const UploadGeometryRequest& event)
    -> void {
  Log.trace("Handling UploadGeometryRequest, id={}, batchId={}", event.requestId, event.batchId);

  const auto uploadInfoList = splitGeometry(event);

  auto uploadResults = std::vector<std::optional<BufferRegion>>{};
  for (const auto& uploadInfo : uploadInfoList) {
    uploadResults.push_back(
        bufferSystem->tryInsert(uploadInfo.dstBuffer, uploadInfo.srcData, uploadInfo.size));
  }

  eventQueue->emit(UploadGeometryResponse{
      .batchId = event.batchId,
      .requestId = event.requestId,
  });
}

auto DefaultUploadSystem::splitGeometry(const UploadGeometryRequest& event)
    -> std::vector<UploadInfo> {
  return {};
}

auto DefaultUploadSystem::handleUploadImage(const UploadImageRequest& event) -> void {
  Log.trace("Handling UploadImageRequest id={}", event.requestId);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  eventQueue->emit(UploadImageResponse{
      .requestId = event.requestId,
  });
}

}
