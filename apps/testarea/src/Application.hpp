#pragma once

#include "IContext.hpp"
#include "IWindow.hpp"

class Application {
 public:
   explicit Application(std::shared_ptr<IWindow> newWindow, std::shared_ptr<IContext> newContext);

 private:
   std::shared_ptr<IWindow> window;
   std::shared_ptr<IContext> context;
};