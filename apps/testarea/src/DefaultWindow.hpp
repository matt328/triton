#pragma once

#include "IWindow.hpp"
class DefaultWindow : public IWindow {
 public:
   DefaultWindow();
   void doWindowThing() override;
   void registerEventBus(std::shared_ptr<IEventBus> newEventBus) override;

 private:
   std::shared_ptr<IEventBus> eventBus;
};