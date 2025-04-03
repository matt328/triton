#include "WidgetService.hpp"
#include "EntityService.hpp"
#include "api/fx/IResourceProxy.hpp"
#include "bk/TaskQueue.hpp"
#include "components/BoxWidget.hpp"
#include "components/TagComponent.hpp"
#include "components/Target.hpp"
#include "api/fx/GeometryGenerator.hpp"

namespace tr {

WidgetService::WidgetService(std::shared_ptr<EntityService> newEntityService,
                             std::shared_ptr<TaskQueue> newTaskQueue,
                             std::shared_ptr<IResourceProxy> newResourceProxy,
                             std::shared_ptr<GeometryGenerator> newGeometryGenerator)
    : entityService{std::move(newEntityService)},
      taskQueue{std::move(newTaskQueue)},
      resourceProxy{std::move(newResourceProxy)},
      geometryGenerator{std::move(newGeometryGenerator)} {
}

auto WidgetService::createBox(const BoxCreateInfo& createInfo) -> void {
  const auto task = [this, &createInfo] {
    // TODO(matt): don't pass entire boxcreateinfo here, only pass what's needed, center and extent
    // create a BoxGeometryCreateInfo struct in GeometryGenerator.hpp
    const auto geometryData = geometryGenerator->generateBox(createInfo);
    const auto meshHandle = resourceProxy->uploadGeometry(geometryData);
    return meshHandle;
  };

  const auto onComplete = [this, &createInfo](MeshHandle meshHandle) {
    entityService->exclusiveAccess(
        [createInfo, meshHandle](std::unique_ptr<entt::registry>& registry) {
          const auto entityId = registry->create();
          if (createInfo.target) {
            const auto targetEntityId = static_cast<entt::entity>(*createInfo.target);
            if (createInfo.targetOffset) {
              registry->emplace<Target>(entityId, targetEntityId, *createInfo.targetOffset);
            } else {
              registry->emplace<Target>(entityId, targetEntityId);
            }
          }
          Log.trace("Widget Service Creating Box");
          registry->emplace<Tag>(entityId, createInfo.tag);
          registry->emplace<BoxWidget>(entityId, meshHandle);
        });
  };

  taskQueue->enqueue(task, onComplete);
}

auto WidgetService::toggleByTag([[maybe_unused]] const std::string& tag,
                                [[maybe_unused]] bool visible) -> void {
}

auto WidgetService::removeByTag([[maybe_unused]] const std::string& tag) -> void {
}

}
