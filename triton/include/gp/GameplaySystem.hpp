#pragma once

#include <entt/fwd.hpp>
#include <entt/signal/delegate.hpp>

#include "gfx/RenderData.hpp"

namespace tr::util {
   class Timer;
}

namespace tr::gfx {
   class RenderContext;
}

namespace tr::gfx::tx {
   class ResourceManager;
}

namespace tr::gp {
   class ActionSystem;
}

namespace tr::ctx {
   class GameplayFacade;
}

namespace tr::gp {
   using EntityType = entt::entity;
   using RenderDataProducer = entt::delegate<void(gfx::RenderData&)>;

   class GameplaySystem {
    public:
      GameplaySystem();
      ~GameplaySystem();

      GameplaySystem(const GameplaySystem&) = delete;
      GameplaySystem(GameplaySystem&&) = delete;
      GameplaySystem& operator=(const GameplaySystem&) = delete;
      GameplaySystem& operator=(GameplaySystem&&) = delete;

      void fixedUpdate(const util::Timer& timer);
      void update(const double blendingFactor);
      void resize(const std::pair<uint32_t, uint32_t>);

      // Input Handlers
      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

      template <auto Candidate, typename Type>
      void addRenderDataListener(Type* valueOrInstance) noexcept {
         renderDataProducer.connect<Candidate>(valueOrInstance);
      }

    private:
      friend class ctx::GameplayFacade;

      std::unique_ptr<entt::registry> registry;

      std::unique_ptr<ActionSystem> actionSystem;

      /// This is a preallocated RenderData that the ECS collects all of he GameWorld data into
      /// before the Renderer takes a copy of it
      gfx::RenderData renderData{};

      // This delegate seems overengineered, but keeps the Application from having to #include
      // half the engine
      RenderDataProducer renderDataProducer{};
   };
}
