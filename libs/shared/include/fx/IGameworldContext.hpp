#pragma once

namespace tr {

class IGameplaySystem;
class IEntityServiceProvider;

class IGameworldContext {
public:
  IGameworldContext() = default;
  virtual ~IGameworldContext() = default;

  IGameworldContext(const IGameworldContext&) = default;
  IGameworldContext(IGameworldContext&&) = delete;
  auto operator=(const IGameworldContext&) -> IGameworldContext& = default;
  auto operator=(IGameworldContext&&) -> IGameworldContext& = delete;

  virtual auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> = 0;
  virtual auto getEntityServiceProvider() -> std::shared_ptr<IEntityServiceProvider> = 0;
};

}
