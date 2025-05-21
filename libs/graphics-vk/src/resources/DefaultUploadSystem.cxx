#include "DefaultUploadSystem.hpp"
#include "api/fx/IEventQueue.hpp"

namespace tr {

DefaultUploadSystem::DefaultUploadSystem(std::shared_ptr<IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  Log.trace("Constructing DefaultUploadSystem");

  eventQueue->subscribe<UploadGeometryRequest>(
      [this](const UploadGeometryRequest& event) { handleUploadGeometry(event); });

  eventQueue->subscribe<UploadImageRequest>(
      [this](const UploadImageRequest& event) { handleUploadImage(event); });
}

auto DefaultUploadSystem::handleUploadGeometry([[maybe_unused]] const UploadGeometryRequest& event)
    -> void {
  Log.trace("Handling UploadGeometryRequest Id={}", event.requestId);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  eventQueue->emit(UploadGeometryResponse{
      .requestId = event.requestId,
  });
}

auto DefaultUploadSystem::handleUploadImage(const UploadImageRequest& event) -> void {
  Log.trace("Handling UploadImageRequest id={}", event.requestId);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  eventQueue->emit(UploadImageResponse{
      .requestId = event.requestId,
  });
}

}
