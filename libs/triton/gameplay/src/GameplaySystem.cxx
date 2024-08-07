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
#include "systems/RenderDataSystem.hpp"

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
   };

   void GameplaySystem::fixedUpdate(const cm::Timer& timer) {
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

      renderDataFn(renderData);
   }

   void GameplaySystem::resize(const std::pair<uint32_t, uint32_t> size) {
      entitySystem->writeWindowDimensions(size);
   }

   void GameplaySystem::keyCallback(cm::Key key, cm::ButtonState buttonState) {
      actionSystem->keyCallback(key, buttonState);
   }

   void GameplaySystem::cursorPosCallback(double xpos, double ypos) {
      actionSystem->cursorPosCallback(xpos, ypos);
   }

   void GameplaySystem::mouseButtonCallback(int button, int action, int mods) {
      actionSystem->mouseButtonCallback(button, action, mods);
   }

   void GameplaySystem::setMouseState(bool captured) {
      actionSystem->setMouseState(captured);
   }

   auto GameplaySystem::createTerrain(const cm::ModelData handles) -> cm::EntityType {
      ZoneNamedN(n, "gameplaySystem.createTerrain", true);
      return entitySystem->createTerrain(handles);
   }

   auto GameplaySystem::createStaticModel(cm::ModelData meshes) -> cm::EntityType {
      return entitySystem->createStaticModel(meshes);
   }

   auto GameplaySystem::createAnimatedModel(cm::ModelData modelData,
                                            const std::filesystem::path& skeletonPath,
                                            const std::filesystem::path& animationPath)
       -> cm::EntityType {
      modelData.animationData =
          cm::AnimationData{.skeletonHandle = animationFactory->loadSkeleton(skeletonPath),
                            .animationHandle = animationFactory->loadAnimation(animationPath)};

      return entitySystem->createAnimatedModel(modelData);
   }

   auto GameplaySystem::createCamera(uint32_t width,
                                     uint32_t height,
                                     float fov,
                                     float zNear,
                                     float zFar,
                                     glm::vec3 position,
                                     std::optional<std::string> name) -> cm::EntityType {
      return entitySystem->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplaySystem::setCurrentCamera(cm::EntityType currentCamera) {
      entitySystem->setCurrentCamera(currentCamera);
   }

   void GameplaySystem::clearEntities() {
      entitySystem->removeAll();
   }
}