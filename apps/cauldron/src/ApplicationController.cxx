#include "ApplicationController.hpp"
#include "api/fx/IEventQueue.hpp"

namespace ed {

ApplicationController::ApplicationController(std::shared_ptr<tr::IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
  eventQueue->subscribe<tr::AddStaticModel>(
      [&](const auto& event) { handleAddStaticModel(event); });

  eventQueue->subscribe<tr::AddStaticGeometry>(
      [&](const auto& event) { handleAddStaticGeometry(event); });

  eventQueue->subscribe<tr::StaticModelUploaded>(
      [&](const auto& event) { handleStaticModelUploaded(event); });
}

auto ApplicationController::handleAddStaticModel(const std::shared_ptr<tr::AddStaticModel>& event)
    -> void {
  Log.trace("ApplicationController, addStaticModel name={}, modelName={}",
            event->name,
            event->modelName);

  const auto beginBatch = tr::BeginResourceBatch{.batchId = 1};
  const auto endBatch = tr::EndResourceBatch{.batchId = 1};

  eventQueue->emit(beginBatch, UIGroup);
  for (int i = 0; i < 3000; ++i) {
    const auto staticModelRequestId = bk::RandomUtils::uint64InRange(1, 10000);

    const auto modelRequest =
        tr::StaticModelRequest{.batchId = 1,
                               .requestId = staticModelRequestId,
                               .modelFilename = event->modelName,
                               .entityName = std::format("{}-{}", event->name, i)};
    inFlightMap.emplace(
        staticModelRequestId,
        tr::GameObjectData{.name = modelRequest.entityName,
                           .orientation = {.position = bk::RandomUtils::vec3InRange(-10, 10)},
                           .modelName = event->modelName,
                           .skeleton = "",
                           .animations = {}});
    eventQueue->emit(modelRequest, UIGroup);
  }

  eventQueue->emit(endBatch, UIGroup);
}

auto ApplicationController::handleStaticModelUploaded(
    const std::shared_ptr<tr::StaticModelUploaded>& event) -> void {
  auto gameObjectData = inFlightMap.at(event->requestId);
  const auto sgo = tr::CreateStaticGameObject{.entityName = event->entityName,
                                              .geometryHandle = event->geometryHandle,
                                              .textureHandle = event->textureHandle,
                                              .gameObjectData = gameObjectData};
  inFlightMap.erase(event->requestId);
  eventQueue->emit(sgo);
}

auto ApplicationController::handleAddStaticGeometry(
    const std::shared_ptr<tr::AddStaticGeometry>& event) -> void {
  Log.trace("ApplicationController, addStaticGeometry name={}", event->name);

  const auto beginBatch = tr::BeginResourceBatch{.batchId = 1};
  const auto endBatch = tr::EndResourceBatch{.batchId = 1};
  const auto staticGeometryRequestId = requestIdGenerator.getKey();

  const auto meshRequest = tr::StaticMeshRequest{.batchId = 1,
                                                 .requestId = staticGeometryRequestId,
                                                 .geometryData = event->geometryData,
                                                 .entityName = event->name};
  inFlightMap.emplace(staticGeometryRequestId,
                      tr::GameObjectData{.name = event->name,
                                         .orientation = event->orientation,
                                         .skeleton = "",
                                         .animations = {}});
  eventQueue->emit(beginBatch, UIGroup);
  eventQueue->emit(meshRequest, UIGroup);
  eventQueue->emit(endBatch, UIGroup);
}

}
