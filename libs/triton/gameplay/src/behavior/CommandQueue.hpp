#pragma once

#include <entt/entity/fwd.hpp>
namespace tr::gp {
   class ICommand {
    public:
      ICommand() = default;
      ICommand(const ICommand&) = default;
      ICommand(ICommand&&) = delete;
      auto operator=(const ICommand&) -> ICommand& = default;
      auto operator=(ICommand&&) -> ICommand& = delete;

      virtual ~ICommand() = default;
      virtual void execute(entt::registry& registry) const = 0;
   };

   class CommandQueue {
      std::queue<std::unique_ptr<ICommand>> commands;

    public:
      void enqueue(std::unique_ptr<ICommand> command) {
         commands.push(std::move(command));
      }

      void processCommands(entt::registry& registry) {
         while (!commands.empty()) {
            commands.front()->execute(registry);
            commands.pop();
         }
      }
   };
}