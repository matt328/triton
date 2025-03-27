#pragma once

#include "api/action/Sources.hpp"
#include "api/action/Actions.hpp"

namespace tr {

class IActionSystem {
public:
  explicit IActionSystem() = default;
  virtual ~IActionSystem() = default;

  IActionSystem(const IActionSystem&) = default;
  IActionSystem(IActionSystem&&) = delete;
  auto operator=(const IActionSystem&) -> IActionSystem& = default;
  auto operator=(IActionSystem&&) -> IActionSystem& = delete;

  virtual void mapSource(Source source, tr::StateType sType, tr::ActionType aType) = 0;
};

}
