#pragma once

namespace tr {

   class IWindow;

   class IContext {
    public:
      virtual ~IContext() = default;

      virtual void setWindow(std::shared_ptr<IWindow> newWindow) = 0;
      virtual void run() = 0;
   };
}