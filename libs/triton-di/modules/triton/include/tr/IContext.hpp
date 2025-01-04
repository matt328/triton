#pragma once

namespace tr {

class IGuiSystem;
class IEventBus;

class IGameplaySystem;

class IContext {
public:
  IContext() = default;
  virtual ~IContext() = default;

  IContext(const IContext&) = default;
  IContext(IContext&&) = delete;
  auto operator=(const IContext&) -> IContext& = default;
  auto operator=(IContext&&) -> IContext& = delete;

  virtual void run() = 0;

  virtual auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> = 0;
  virtual auto getGuiSystem() -> std::shared_ptr<IGuiSystem> = 0;
  virtual auto getEventSystem() -> std::shared_ptr<IEventBus> = 0;
};

}
