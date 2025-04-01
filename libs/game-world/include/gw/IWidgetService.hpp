#pragma once

#include "api/gw/GameObjectType.hpp"
#include "bk/Color.hpp"

namespace tr {

struct BoxCreateInfo {
  std::string tag;
  glm::vec3 center;
  float extent;
  Color color{Colors::White};
  std::optional<GameObjectId> target{std::nullopt};
  std::optional<glm::vec3> targetOffset{std::nullopt};
};

class IWidgetService {
public:
  IWidgetService() = default;
  virtual ~IWidgetService() = default;

  IWidgetService(const IWidgetService&) = default;
  IWidgetService(IWidgetService&&) = delete;
  auto operator=(const IWidgetService&) -> IWidgetService& = default;
  auto operator=(IWidgetService&&) -> IWidgetService& = delete;

  virtual auto createBox(const BoxCreateInfo& createInfo) -> void = 0;

  virtual auto toggleByTag(const std::string& tag, bool visible) -> void = 0;

  virtual auto removeByTag(const std::string& tag) -> void = 0;
};

}
