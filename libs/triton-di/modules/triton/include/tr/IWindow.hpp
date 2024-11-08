#pragma once

#include "IEventBus.hpp"

namespace tr {
   class IWindow {
    public:
      IWindow() = default;
      virtual ~IWindow() = default;

      IWindow(const IWindow&) = default;
      IWindow(IWindow&&) = delete;
      auto operator=(const IWindow&) -> IWindow& = default;
      auto operator=(IWindow&&) -> IWindow& = delete;

      virtual auto getNativeWindow() -> void* = 0;
      virtual void pollEvents() = 0;
      virtual void registerEventBus(std::shared_ptr<IEventBus> newEventBus) = 0;
   };
}