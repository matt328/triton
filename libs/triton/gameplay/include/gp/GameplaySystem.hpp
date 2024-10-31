#pragma once

#include "cm/EntitySystemTypes.hpp"

#include "cm/Handles.hpp"

#include "cm/Inputs.hpp"

#include "cm/RenderData.hpp"

namespace tr::cm {
   class Timer;
} // namespace tr::cm

namespace tr::gfx {
   class RenderContext;
} // namespace tr::gfx

namespace tr::gfx::tx {
   class ResourceManager;
} // namespace tr::gfx::tx

namespace tr::ctx {
   class GameplayFacade;
} // namespace tr::ctx

namespace tr::gp {
   using RenderDataProducer = entt::delegate<void(cm::gpu::RenderData&)>;
   using RenderDataTransferHandler = std::function<void(cm::gpu::RenderData&)>;

   class AnimationFactory;
   class EntitySystem;
   class ActionSystem;

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
   */
   class GameplaySystem {
    public:
      GameplaySystem();
      ~GameplaySystem();

      GameplaySystem(const GameplaySystem&) = delete;
      GameplaySystem(GameplaySystem&&) = delete;
      auto operator=(const GameplaySystem&) -> GameplaySystem& = delete;
      auto operator=(GameplaySystem&&) -> GameplaySystem& = delete;

      void fixedUpdate(const cm::Timer& timer) const;
      void update(double blendingFactor);
      void resize(std::pair<uint32_t, uint32_t>) const;

      // Input Handlers
      void keyCallback(cm::Key key, cm::ButtonState buttonState) const;
      void cursorPosCallback(double xpos, double ypos) const;
      void mouseButtonCallback(int button, int action, int mods) const;
      void setMouseState(bool captured) const;

      // Gameworld State Methods
      [[nodiscard]] auto createTerrain(const std::vector<cm::ModelData>& handles) const
          -> cm::EntityType;
      [[nodiscard]] auto createStaticModel(const cm::ModelData& meshes) const -> cm::EntityType;
      [[nodiscard]] auto createAnimatedModel(cm::ModelData modelData,
                                             const std::filesystem::path& skeletonPath,
                                             const std::filesystem::path& animationPath) const
          -> cm::EntityType;
      [[nodiscard]] auto createCamera(uint32_t width,
                                      uint32_t height,
                                      float fov,
                                      float zNear,
                                      float zFar,
                                      const glm::vec3& position,
                                      const std::optional<std::string>& name) const
          -> cm::EntityType;

      [[nodiscard]] auto createDebugAABB(const glm::vec3& min,
                                         const glm::vec3& max) const -> cm::EntityType;
      [[nodiscard]] auto createDebugTriangle(const std::array<glm::vec3, 3> vertices) const
          -> cm::EntityType;
      [[nodiscard]] auto createDebugLine(const glm::vec3& start,
                                         const glm::vec3& end) const -> cm::EntityType;
      [[nodiscard]] auto createDebugPoint(const glm::vec3& position) const -> cm::EntityType;

      void hideEntity(cm::EntityType);
      void showEntity(cm::EntityType);
      void hideDebugEntities();
      void showDebugEntities();

      void setCurrentCamera(cm::EntityType currentCamera) const;
      void clearEntities() const;

      void setRenderDataTransferHandler(const RenderDataTransferHandler& func) {
         transferRenderData = func;
      }

    private:
      std::unique_ptr<EntitySystem> entitySystem;
      std::unique_ptr<ActionSystem> actionSystem;
      std::unique_ptr<AnimationFactory> animationFactory;

      /// This is a preallocated RenderData that the ECS collects all of he GameWorld data into
      /// before the Renderer takes a copy of it
      cm::gpu::RenderData renderData{};

      RenderDataTransferHandler transferRenderData{};
   };
} // namespace tr::gp
