#include "BehaviorSystem.hpp"

#include <entt/entity/fwd.hpp>

#include "behavior/commands/CreateTerrain.hpp"
#include "cm/Handles.hpp"

#include "components/Animation.hpp"
#include "components/DebugAABB.hpp"
#include "components/Resources.hpp"
#include "components/Renderable.hpp"
#include "components/Transform.hpp"
#include "components/Camera.hpp"
#include "components/DebugLine.hpp"

#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/AnimationSystem.hpp"

namespace tr::gp {
   BehaviorSystem::BehaviorSystem() {
      registry = std::make_unique<entt::registry>();
      commandQueue = std::make_unique<CommandQueue>();
   }

   BehaviorSystem::~BehaviorSystem() {
   }

   void BehaviorSystem::fixedUpdate([[maybe_unused]] const cm::Timer& timer,
                                    const AnimationFactory& animationFactory) {
      std::unique_lock lock{registryMutex};
      commandQueue->processCommands(*registry);
      {
         ZoneNamedN(camZone, "CameraSystem", true);
         sys::CameraSystem::fixedUpdate(*registry);
      }

      {
         ZoneNamedN(xformZone, "TransformSystem", true);
         sys::TransformSystem::update(*registry);
      }

      {
         ZoneNamedN(animationZone, "AnimationSystem", true);
         sys::AnimationSystem::update(*registry, animationFactory);
      }
   }

   void BehaviorSystem::prepareRenderData(cm::gpu::RenderData& renderData) {
      auto lock = std::shared_lock{registryMutex};
      sys::RenderDataSystem::update(*registry, renderData);
   }

   void BehaviorSystem::writeCameras(const std::function<void(entt::entity, cmp::Camera)>& fn) {
      auto lock = std::unique_lock{registryMutex};
      registry->view<cmp::Camera>().each(fn);
   }

   void BehaviorSystem::writeCameras(
       std::function<void(entt::entity, cmp::Camera, uint32_t width, uint32_t height)> fn) {

      auto lock = std::unique_lock{registryMutex};
      const auto [width, height] = registry->ctx().get<const cmp::WindowDimensions>();
      registry->view<cmp::Camera>().each(
          [&width, &height, &fn](auto entity, auto cam) { fn(entity, cam, width, height); });
   }

   void BehaviorSystem::writeWindowDimensions(const std::pair<uint32_t, uint32_t> size) {
      auto lock = std::unique_lock{registryMutex};
      registry->ctx().insert_or_assign<cmp::WindowDimensions>(
          cmp::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

   void BehaviorSystem::createTerrain(const std::vector<cm::ModelData>& handles) {
      commandQueue->enqueue(
          std::make_unique<CreateTerrainCommand>(handles, terrainCreatedListeners));
   }

   auto BehaviorSystem::createStaticModel(const cm::ModelData& handles) -> cm::EntityType {
      auto lock = std::unique_lock{registryMutex};
      const auto e = registry->create();
      registry->emplace<cmp::Renderable>(e, std::vector{handles.meshData});
      registry->emplace<cmp::Transform>(e);
      return e;
   }

   auto BehaviorSystem::createAnimatedModel(const cm::ModelData& modelData) -> cm::EntityType {
      auto lock = std::unique_lock{registryMutex};
      const auto modelEntity = registry->create();

      if (!modelData.animationData) {
         // TODO(matt) Better Exceptions here
         throw std::runtime_error("Animated Model created without animation data");
      }

      if (!modelData.skinData) {
         throw std::runtime_error("Animated Model created without skin data");
      }

      registry->emplace<cmp::Animation>(modelEntity,
                                        modelData.animationData->animationHandle,
                                        modelData.animationData->skeletonHandle,
                                        modelData.skinData->jointMap,
                                        modelData.skinData->inverseBindMatrices);
      registry->emplace<cmp::Transform>(modelEntity);

      registry->emplace<cmp::Renderable>(modelEntity, std::vector{modelData.meshData});

      return modelEntity;
   }

   auto BehaviorSystem::createCamera(uint32_t width,
                                     uint32_t height,
                                     float fov,
                                     float zNear,
                                     float zFar,
                                     glm::vec3 position,
                                     [[maybe_unused]] const std::optional<std::string>& name)
       -> cm::EntityType {

      auto lock = std::unique_lock{registryMutex};
      const auto camera = registry->create();
      registry->emplace<cmp::Camera>(camera, width, height, fov, zNear, zFar, position);
      return camera;
   }

   void BehaviorSystem::setCurrentCamera(const cm::EntityType currentCamera) {
      auto lock = std::unique_lock{registryMutex};
      registry->ctx().insert_or_assign<cmp::CurrentCamera>(cmp::CurrentCamera{currentCamera});
   }

   [[nodiscard]] auto BehaviorSystem::createDebugAABB(const glm::vec3& min, const glm::vec3& max)
       -> cm::EntityType {
      auto lock = std::unique_lock{registryMutex};
      // TODO(matt) hook this up tomorrow.
      /*
         create the AABB geometry, add it as a renderable with the line list topology and that
         should do it
      */
      const auto boxEntity = registry->create();
      registry->emplace<cmp::DebugAABB>(boxEntity, min, max);
      return boxEntity;
   }

   [[nodiscard]] auto BehaviorSystem::createDebugTriangle(
       const std::array<glm::vec3, 3> vertices) const -> cm::EntityType {
   }

   [[nodiscard]] auto BehaviorSystem::createDebugLine(const glm::vec3& start, const glm::vec3& end)
       -> cm::EntityType {
      auto lock = std::unique_lock{registryMutex};
      const auto lineEntity = registry->create();
      registry->emplace<cmp::DebugLine>(lineEntity, start, end);
      return lineEntity;
   }

   [[nodiscard]] auto BehaviorSystem::createDebugPoint(const glm::vec3& position) const
       -> cm::EntityType {
   }

   void BehaviorSystem::removeAll() {
      auto lock = std::unique_lock{registryMutex};
      registry->clear();
      registry->ctx().erase<cmp::CurrentCamera>();
   }

}