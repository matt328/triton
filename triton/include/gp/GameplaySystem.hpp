#pragma once

#include <entt/fwd.hpp>
#include <entt/signal/delegate.hpp>

#include "gfx/RenderObject.hpp"
#include "gfx/ObjectData.hpp"

namespace tr::util {
   class Timer;
}

namespace tr::gfx {
   class RenderContext;
}

namespace tr::gp {
   class ActionSystem;
}

namespace tr::ctx {
   class GameplayFacade;
}

namespace tr::gp {
   using EntityType = entt::entity;

   using RenderObjectProducer = entt::delegate<void(gfx::RenderObject)>;
   using CameraDataProducer = entt::delegate<void(gfx::CameraData)>;

   class GameplaySystem {
    public:
      GameplaySystem();
      ~GameplaySystem();

      GameplaySystem(const GameplaySystem&) = delete;
      GameplaySystem(GameplaySystem&&) = delete;
      GameplaySystem& operator=(const GameplaySystem&) = delete;
      GameplaySystem& operator=(GameplaySystem&&) = delete;

      void fixedUpdate(const util::Timer& timer);
      void update();
      void resize(const std::pair<uint32_t, uint32_t>);

      // Input Handlers
      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

      template <auto Candidate, typename Type>
      void addRenderObjectListener(Type* valueOrInstance) noexcept {
         renderObjectProducer.connect<Candidate>(valueOrInstance);
      }

      template <auto Candidate, typename Type>
      void addCameraDataListener(Type* valueOrInstance) noexcept {
         cameraDataProducer.connect<Candidate>(valueOrInstance);
      }

    private:
      friend class ctx::GameplayFacade;

      std::unique_ptr<entt::registry> registry;
      std::unique_ptr<ActionSystem> actionSystem;

      RenderObjectProducer renderObjectProducer{};
      CameraDataProducer cameraDataProducer{};
   };
}
