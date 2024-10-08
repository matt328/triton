#include "gp/GameplaySystem.hpp"

#include "cm/Handles.hpp"
#include "cm/Inputs.hpp"

#include "EntitySystem.hpp"
#include "AnimationFactory.hpp"

#include "actions/ActionType.hpp"
#include "actions/ActionSystem.hpp"
#include "actions/Action.hpp"
#include "actions/Sources.hpp"

#include "systems/CameraSystem.hpp"

namespace tr::gp {

   /*
      TODO: Either make AnimationSystem a class with a reference to the AnimationFactory
      or make this class hold a reference to the AnimationFactory and pass it into
      AnimationSystem::update()
   */

   GameplaySystem::GameplaySystem() {

      entitySystem = std::make_unique<EntitySystem>();
      actionSystem = std::make_unique<ActionSystem>();
      animationFactory = std::make_unique<AnimationFactory>();

      using cm::Key;

      // TODO: synchronization isn't applied correctly (at all) here.
      actionSystem->getDelegate().connect<&sys::CameraSystem::handleAction>(
          *entitySystem->getRegistry());

      // Forward
      actionSystem->mapSource(Source{cm::Key::Up, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveForward);
      actionSystem->mapSource(Source{cm::Key::W, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveForward);

      // Backward
      actionSystem->mapSource(Source{cm::Key::Down, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveBackward);
      actionSystem->mapSource(Source{cm::Key::S, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveBackward);
      // Left
      actionSystem->mapSource(Source{cm::Key::Left, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeLeft);
      actionSystem->mapSource(Source{cm::Key::A, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeLeft);
      // Right
      actionSystem->mapSource(Source{cm::Key::Right, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeRight);
      actionSystem->mapSource(Source{cm::Key::D, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeRight);
      // Look
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_X, SourceType::Float},
                              StateType::Range,
                              ActionType::LookHorizontal);
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_Y, SourceType::Float},
                              StateType::Range,
                              ActionType::LookVertical);
   }

   GameplaySystem::~GameplaySystem() {
      Log.info("Destroying Game");
   }

   void GameplaySystem::fixedUpdate(const cm::Timer& timer) const {
      ZoneNamedN(upd, "FixedUpdate", true);
      entitySystem->fixedUpdate(timer, *animationFactory);
   }

   void GameplaySystem::update(const double blendingFactor) {
      TracyPlot("Physics Blend Factor", blendingFactor);
      ZoneNamedN(upd, "Update", true);

      renderData.objectData.clear();
      renderData.staticMeshData.clear();
      renderData.terrainMeshData.clear();
      renderData.skinnedMeshData.clear();
      renderData.animationData.clear();

      entitySystem->prepareRenderData(renderData);

      transferRenderData(renderData);
   }

   void GameplaySystem::resize(const std::pair<uint32_t, uint32_t> size) const {
      entitySystem->writeWindowDimensions(size);
   }

   void GameplaySystem::keyCallback(const cm::Key key, const cm::ButtonState buttonState) const {
      actionSystem->keyCallback(key, buttonState);
   }

   void GameplaySystem::cursorPosCallback(const double xpos, const double ypos) const {
      actionSystem->cursorPosCallback(xpos, ypos);
   }

   void GameplaySystem::mouseButtonCallback(const int button,
                                            const int action,
                                            const int mods) const {
      actionSystem->mouseButtonCallback(button, action, mods);
   }

   void GameplaySystem::setMouseState(const bool captured) const {
      actionSystem->setMouseState(captured);
   }

   auto GameplaySystem::createTerrain(const cm::ModelData& handles) const -> cm::EntityType {
      ZoneNamedN(n, "gameplaySystem.createTerrain", true);
      return entitySystem->createTerrain(handles);
   }

   auto GameplaySystem::createStaticModel(const cm::ModelData& meshes) const -> cm::EntityType {
      return entitySystem->createStaticModel(meshes);
   }

   auto GameplaySystem::createAnimatedModel(cm::ModelData modelData,
                                            const std::filesystem::path& skeletonPath,
                                            const std::filesystem::path& animationPath) const
       -> cm::EntityType {
      modelData.animationData =
          cm::AnimationData{.skeletonHandle = animationFactory->loadSkeleton(skeletonPath),
                            .animationHandle = animationFactory->loadAnimation(animationPath)};

      return entitySystem->createAnimatedModel(modelData);
   }

   auto GameplaySystem::createCamera(const uint32_t width,
                                     const uint32_t height,
                                     const float fov,
                                     const float zNear,
                                     const float zFar,
                                     const glm::vec3& position,
                                     const std::optional<std::string>& name) const
       -> cm::EntityType {
      return entitySystem->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplaySystem::setCurrentCamera(const cm::EntityType currentCamera) const {
      entitySystem->setCurrentCamera(currentCamera);
   }

   void GameplaySystem::clearEntities() const {
      entitySystem->removeAll();
   }

   [[nodiscard]] auto GameplaySystem::createDebugAABB(const glm::vec3& min,
                                                      const glm::vec3& max) const
       -> cm::EntityType {
      return entitySystem->createDebugAABB(min, max);
   }

   [[nodiscard]] auto GameplaySystem::createDebugTriangle(
       const std::array<glm::vec3, 3> vertices) const -> cm::EntityType {
      return entitySystem->createDebugTriangle(vertices);
   }

   [[nodiscard]] auto GameplaySystem::createDebugLine(const glm::vec3& start,
                                                      const glm::vec3& end) const
       -> cm::EntityType {
      return entitySystem->createDebugLine(start, end);
   }

   [[nodiscard]] auto GameplaySystem::createDebugPoint(const glm::vec3& position) const
       -> cm::EntityType {
      return entitySystem->createDebugPoint(position);
   }

}