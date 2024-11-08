#pragma once

class IWindow;

class IContext {
 public:
   virtual ~IContext() = default;

   virtual void setWindow(std::shared_ptr<IWindow> window) = 0;
};