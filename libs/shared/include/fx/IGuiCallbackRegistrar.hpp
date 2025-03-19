#pragma once

namespace tr {
class IGuiCallbackRegistrar {
public:
  IGuiCallbackRegistrar() = default;
  virtual ~IGuiCallbackRegistrar() = default;

  IGuiCallbackRegistrar(const IGuiCallbackRegistrar&) = default;
  IGuiCallbackRegistrar(IGuiCallbackRegistrar&&) = delete;
  auto operator=(const IGuiCallbackRegistrar&) -> IGuiCallbackRegistrar& = default;
  auto operator=(IGuiCallbackRegistrar&&) -> IGuiCallbackRegistrar& = delete;

  virtual auto setRenderCallback(std::function<void(void)> newRenderFn) -> void = 0;
};
}

/*
  This interface is in shared so both framework and graphics can include it.
  The Implementation will be defined in framework.
  When framework creates the framework context, it'll create an Implementation, and register it with
  the framework's injector.
  Graphics will have its own factory, which creates its internal injector.
  This factory will take in the frameworkContext, and use the di factory to register things created
  by the framework with Graphics' injector.

  How it passes things into the Graphics factory will be difficult because framework will depend on
  graphics, meaning framework can import graphics' factory class, but graphics can't import
  IFrameWorkContext, unless we move IFrameworkContext into shared.
*/
