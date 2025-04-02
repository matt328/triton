#pragma once

#include "api/gw/BoxCreateInfo.hpp"

namespace tr {

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
