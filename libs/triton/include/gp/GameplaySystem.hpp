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

namespace tr::gfx::geo {
   class AnimationFactory;
}

namespace tr::gfx::tx {
   class ResourceManager;
}

namespace tr::gp {
   class EntitySystem;
   class ActionSystem;
}

namespace tr::ctx {
   class GameplayFacade;
}

namespace tr::gp {
   using RenderDataProducer = entt::delegate<void(gfx::RenderData&)>;

   /*
      This class serves as the API of the gp module of the engine.
      It recieves update and resize and input information from the app module.
      It owns the EntitySystem, ActionSystem, and also for some reason the AnimationFactory.

      The whole point of the gp module is to operate on the EntitySystem. The gfx module then
      queries the gp module at the sync point to determine what to render.

      For now, the Editor directly drives the gp module by creating Entities and setting their
      properties and the gfx module renders them.

      At some point a layer in the gp module will drive itself via properties set on the Entities in
      the EntitySystem, these will be entity controllers in the form of Behaviors, Scripts, etc
      The Editor will then only create entities, set properties, and then be able to start, stop,
      and reset the timestep.

      // TODO move animation factory into ResourceManager
   */
   class GameplaySystem {
    public:
      GameplaySystem(gfx::geo::AnimationFactory& animationFactory);
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

      [[nodiscard]] auto& getAnimationFactory() {
         return animationFactory;
      }

    private:
      friend class ctx::GameplayFacade;

      std::unique_ptr<EntitySystem> entitySystem;
      std::unique_ptr<ActionSystem> actionSystem;

      /*
         TODO: Move RenderData into an intermediate module so it's not part of either gfx or gp
         Probably GameplayFacade belongs there too?
      */
      /// This is a preallocated RenderData that the ECS collects all of he GameWorld data into
      /// before the Renderer takes a copy of it
      gfx::RenderData renderData{};

      // This delegate seems overengineered, but keeps the Application from having to #include
      // half the engine
      RenderDataProducer renderDataProducer{};

      gfx::geo::AnimationFactory& animationFactory;
   };
}
