#pragma once

namespace tr {

class IGameworldContext;

auto createGameworldContext() -> std::shared_ptr<IGameworldContext>;

}
