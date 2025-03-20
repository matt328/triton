#pragma once

namespace tr {

class IGameworldContext;
class IEventBus;

auto createGameworldContext(std::shared_ptr<IEventBus> eventBus)
    -> std::shared_ptr<IGameworldContext>;

}
