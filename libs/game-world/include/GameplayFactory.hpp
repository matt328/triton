#pragma once

namespace tr {

class IGameplayContext;
class IEventBus;

class ComponentFactory {
public:
  static auto getContext(std::shared_ptr<IEventBus> eventBus) -> std::shared_ptr<IGameplayContext>;
};

}
