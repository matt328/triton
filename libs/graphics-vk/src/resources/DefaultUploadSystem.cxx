#include "DefaultUploadSystem.hpp"
#include "api/fx/IEventQueue.hpp"

namespace tr {

DefaultUploadSystem::DefaultUploadSystem(std::shared_ptr<IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  Log.trace("Constructing DefaultUploadSystem");
  eventQueue->subscribe<StaticModelRequest>(
      [this](const StaticModelRequest& event) { handleStaticModelRequest(event); });
}

auto DefaultUploadSystem::handleStaticModelRequest([[maybe_unused]] const StaticModelRequest& event)
    -> void {
}

}
