#pragma once

#include "gw/IWidgetService.hpp"

namespace tr {

class EntityService;

class WidgetService : public IWidgetService {
public:
  explicit WidgetService(std::shared_ptr<EntityService> newEntityService);
  ~WidgetService() override = default;

  WidgetService(const WidgetService&) = delete;
  WidgetService(WidgetService&&) = delete;
  auto operator=(const WidgetService&) -> WidgetService& = delete;
  auto operator=(WidgetService&&) -> WidgetService& = delete;

  auto createBox(const BoxCreateInfo& createInfo) -> void override;

  auto toggleByTag(const std::string& tag, bool visible) -> void override;

  auto removeByTag(const std::string& tag) -> void override;

private:
  std::shared_ptr<EntityService> entityService;
};

}
