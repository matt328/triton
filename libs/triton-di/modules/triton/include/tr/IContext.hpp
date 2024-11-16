#pragma once

namespace tr {

   namespace gp {
      class IGameplaySystem;
   }

   class IContext {
    public:
      IContext() = default;
      virtual ~IContext() = default;

      IContext(const IContext&) = default;
      IContext(IContext&&) = delete;
      auto operator=(const IContext&) -> IContext& = default;
      auto operator=(IContext&&) -> IContext& = delete;

      virtual void run() = 0;

      virtual auto getGameplaySystem() -> std::shared_ptr<gp::IGameplaySystem> = 0;
   };
}