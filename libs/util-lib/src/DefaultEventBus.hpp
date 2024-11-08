#pragma once

#include "IEventBus.hpp"

class DefaultEventBus : public IEventBus {
 public:
   DefaultEventBus();
   void sendEvent() override;
};
