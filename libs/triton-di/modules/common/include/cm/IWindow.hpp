#pragma once

namespace tr::cm {

   class IWindow {
    public:
      IWindow() = default;
      virtual ~IWindow() = default;

      virtual void pollEvents() = 0;

      virtual auto getNativeWindow() -> void* = 0;
   };
}