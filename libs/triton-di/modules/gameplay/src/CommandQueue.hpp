#pragma once

#include <entt/entity/fwd.hpp>

namespace tr::gp {
   template <typename T>
   class ICommand {
    public:
      ICommand() = default;
      ICommand(const ICommand&) = default;
      ICommand(ICommand&&) = delete;
      auto operator=(const ICommand&) -> ICommand& = default;
      auto operator=(ICommand&&) -> ICommand& = delete;

      virtual ~ICommand() = default;
      virtual void execute(T& thing) const = 0;
   };

   template <typename T>
   class CommandQueue {
      std::queue<std::unique_ptr<ICommand<T>>> commands;

    public:
      void enqueue(std::unique_ptr<ICommand<T>> command) {
         commands.push(std::move(command));
      }

      void processCommands(T& thing) {
         while (!commands.empty()) {
            commands.front()->execute(thing);
            commands.pop();
         }
      }
   };
}