#include "ApplicationController.hpp"
#include "api/fx/IEventQueue.hpp"

namespace ed {

ApplicationController::ApplicationController(std::shared_ptr<tr::IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  eventQueue->subscribe<tr::AddStaticModel>(
      [&](const tr::AddStaticModel& event) { handleAddStaticModel(event); });
  eventQueue->subscribe<tr::StaticModelUploaded>(
      [&](const tr::StaticModelUploaded& event) { handleStaticModelUploaded(event); });
}

auto ApplicationController::handleAddStaticModel(const tr::AddStaticModel& event) -> void {
  Log.trace("ApplicationController, addStaticModel name={}, modelName={}",
            event.name,
            event.modelName);

  const auto beginBatch = tr::BeginResourceBatch{.batchId = 1};
  const auto endBatch = tr::EndResourceBatch{.batchId = 1};
  const auto staticModelRequestId = requestIdGenerator.getKey();

  const auto modelRequest = tr::StaticModelRequest{.batchId = 1,
                                                   .requestId = staticModelRequestId,
                                                   .modelFilename = event.modelName,
                                                   .entityName = event.name};
  inFlightMap.emplace(staticModelRequestId,
                      tr::GameObjectData{.name = event.name,
                                         .orientation = event.orientation,
                                         .modelName = event.modelName,
                                         .skeleton = "",
                                         .animations = {}});
  eventQueue->emit(beginBatch, UIGroup);
  eventQueue->emit(modelRequest, UIGroup);
  eventQueue->emit(endBatch, UIGroup);
}

auto ApplicationController::handleStaticModelUploaded(const tr::StaticModelUploaded& event)
    -> void {
  auto gameObjectData = inFlightMap.at(event.requestId);
  const auto sgo = tr::CreateStaticGameObject{.entityName = event.entityName,
                                              .geometryHandle = event.geometryHandle,
                                              .gameObjectData = gameObjectData};
  inFlightMap.erase(event.requestId);
  eventQueue->emit(sgo);
}

}
