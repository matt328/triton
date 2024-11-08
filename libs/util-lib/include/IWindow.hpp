#pragma once

#include "IEventBus.hpp"
class IWindow {
 public:
   virtual ~IWindow() = default;

   virtual void doWindowThing() = 0;
   virtual void registerEventBus(std::shared_ptr<IEventBus> newEventBus) = 0;
};