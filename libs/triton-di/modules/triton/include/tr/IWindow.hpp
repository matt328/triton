#pragma once

#include "IEventBus.hpp"

namespace tr {
   class IWindow {
    public:
      virtual ~IWindow() = default;

      virtual auto getNativeWindow() -> void* = 0;
      virtual void pollEvents() = 0;
      virtual void registerEventBus(std::shared_ptr<IEventBus> newEventBus) = 0;
   };
}