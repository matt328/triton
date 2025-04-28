#pragma once

namespace tr {

class DispatchContext {
public:
  DispatchContext() = default;
  ~DispatchContext() = default;

  DispatchContext(const DispatchContext&) = default;
  DispatchContext(DispatchContext&&) = delete;
  auto operator=(const DispatchContext&) -> DispatchContext& = default;
  auto operator=(DispatchContext&&) -> DispatchContext& = delete;

  auto dispatch() -> void;
};

}
