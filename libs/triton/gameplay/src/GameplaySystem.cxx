#include "gp/GameplaySystem.hpp"

#include "cm/Handles.hpp"
#include "cm/Inputs.hpp"

#include "behavior/BehaviorSystem.hpp"
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

      behaviorSystem = std::make_unique<BehaviorSystem>();
      actionSystem = std::make_unique<ActionSystem>();
      animationFactory = std::make_unique<AnimationFactory>();

      using cm::Key;

      // TODO: synchronization isn't applied correctly (at all) here.
      actionSystem->getDelegate().connect<&sys::CameraSystem::handleAction>(
          *behaviorSystem->getRegistry());

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
      behaviorSystem->fixedUpdate(timer, *animationFactory);
   }

   void GameplaySystem::update(const double blendingFactor) {
      TracyPlot("Physics Blend Factor", blendingFactor);
      ZoneNamedN(upd, "Update", true);

      renderData.objectData.clear();
      renderData.staticMeshData.clear();
      renderData.terrainMeshData.clear();
      renderData.skinnedMeshData.clear();
      renderData.animationData.clear();

      behaviorSystem->prepareRenderData(renderData);

      transferRenderData(renderData);
   }

   void GameplaySystem::resize(const std::pair<uint32_t, uint32_t> size) const {
      behaviorSystem->writeWindowDimensions(size);
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

   void GameplaySystem::createTerrain(const std::vector<cm::ModelData>& handles) const {
      ZoneNamedN(n, "gameplaySystem.createTerrain", true);
      behaviorSystem->createTerrain(handles);
   }

   auto GameplaySystem::createStaticModel(const cm::ModelData& meshes) const -> cm::EntityType {
      return behaviorSystem->createStaticModel(meshes);
   }

   auto GameplaySystem::createAnimatedModel(cm::ModelData modelData,
                                            const std::filesystem::path& skeletonPath,
                                            const std::filesystem::path& animationPath) const
       -> cm::EntityType {
      modelData.animationData =
          cm::AnimationData{.skeletonHandle = animationFactory->loadSkeleton(skeletonPath),
                            .animationHandle = animationFactory->loadAnimation(animationPath)};

      return behaviorSystem->createAnimatedModel(modelData);
   }

   auto GameplaySystem::createCamera(const uint32_t width,
                                     const uint32_t height,
                                     const float fov,
                                     const float zNear,
                                     const float zFar,
                                     const glm::vec3& position,
                                     const std::optional<std::string>& name) const
       -> cm::EntityType {
      return behaviorSystem->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplaySystem::setCurrentCamera(const cm::EntityType currentCamera) const {
      behaviorSystem->setCurrentCamera(currentCamera);
   }

   void GameplaySystem::clearEntities() const {
      behaviorSystem->removeAll();
   }

   [[nodiscard]] auto GameplaySystem::createDebugAABB(const glm::vec3& min,
                                                      const glm::vec3& max) const
       -> cm::EntityType {
      return behaviorSystem->createDebugAABB(min, max);
   }

   [[nodiscard]] auto GameplaySystem::createDebugTriangle(
       const std::array<glm::vec3, 3> vertices) const -> cm::EntityType {
      return behaviorSystem->createDebugTriangle(vertices);
   }

   [[nodiscard]] auto GameplaySystem::createDebugLine(const glm::vec3& start,
                                                      const glm::vec3& end) const
       -> cm::EntityType {
      return behaviorSystem->createDebugLine(start, end);
   }

   [[nodiscard]] auto GameplaySystem::createDebugPoint(const glm::vec3& position) const
       -> cm::EntityType {
      return behaviorSystem->createDebugPoint(position);
   }

   void GameplaySystem::addTerrainCreatedListener(const cm::TerrainCreatedFn& func) {
      behaviorSystem->addTerrainCreatedListener(func);
   }

}