#pragma once

class IEventBus {
 public:
   virtual ~IEventBus() = default;
   virtual void sendEvent() = 0;
};