#include "WidgetService.hpp"
#include "EntityService.hpp"
#include "components/BoxWidget.hpp"
#include "components/TagComponent.hpp"
#include "components/Target.hpp"

namespace tr {

WidgetService::WidgetService(std::shared_ptr<EntityService> newEntityService)
    : entityService{std::move(newEntityService)} {
}

auto WidgetService::createBox(const BoxCreateInfo& createInfo) -> void {
  entityService->exclusiveAccess([createInfo](std::unique_ptr<entt::registry>& registry) {
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
    registry->emplace<BoxWidget>(entityId, createInfo.center, createInfo.extent, createInfo.color);
  });
}

auto WidgetService::toggleByTag([[maybe_unused]] const std::string& tag,
                                [[maybe_unused]] bool visible) -> void {
}

auto WidgetService::removeByTag([[maybe_unused]] const std::string& tag) -> void {
}

}
