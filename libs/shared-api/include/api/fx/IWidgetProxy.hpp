#pragma once

#include "api/gw/GameObjectType.hpp"
#include "bk/Color.hpp"

namespace tr {

class IWidgetProxy {
public:
  IWidgetProxy() = default;
  virtual ~IWidgetProxy() = default;

  IWidgetProxy(const IWidgetProxy&) = default;
  IWidgetProxy(IWidgetProxy&&) = delete;
  auto operator=(const IWidgetProxy&) -> IWidgetProxy& = default;
  auto operator=(IWidgetProxy&&) -> IWidgetProxy& = delete;

  virtual auto createBox(const std::string& tag,
                         glm::vec3 center,
                         float extent,
                         Color color,
                         std::optional<GameObjectId> gameObjectId) -> void = 0;

  virtual auto toggleByTag(const std::string& tag, bool visible) -> void = 0;

  virtual auto removeByTag(const std::string& tag) -> void = 0;
};

}
