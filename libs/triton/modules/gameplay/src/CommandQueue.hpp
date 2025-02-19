#pragma once

#include <entt/entity/fwd.hpp>

namespace tr {
template <typename... Args>
class ICommand {
public:
  ICommand() = default;
  ICommand(const ICommand&) = default;
  ICommand(ICommand&&) = delete;
  auto operator=(const ICommand&) -> ICommand& = default;
  auto operator=(ICommand&&) -> ICommand& = delete;

  virtual ~ICommand() = default;
  virtual void execute(Args... args) const = 0;
};

template <typename... Args>
class CommandQueue {
public:
  void enqueue(std::unique_ptr<ICommand<Args...>> command) {
    commands.push(std::move(command));
  }

  void processCommands(Args... args) {
    while (!commands.empty()) {
      commands.front()->execute(args...);
      commands.pop();
    }
  }

private:
  std::queue<std::unique_ptr<ICommand<Args...>>> commands;
};

template <typename... Args>
class CommandExecutor {
public:
  explicit CommandExecutor(Args&&... args) : storedArgs(std::forward<Args>(args)...) {
  }

  void execute(std::unique_ptr<ICommand<Args...>> command) {
    std::apply(
        [&](auto&&... unpackedArgs) {
          command->execute(std::forward<decltype(unpackedArgs)>(unpackedArgs)...);
        },
        storedArgs);
  }

private:
  std::tuple<Args...> storedArgs;
};

}
