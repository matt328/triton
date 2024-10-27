#include "EntitySystem.hpp"

#include <entt/entity/fwd.hpp>

#include "cm/Handles.hpp"

#include "components/Animation.hpp"
#include "components/DebugAABB.hpp"
#include "components/Resources.hpp"
#include "components/DebugConstants.hpp"
#include "components/Renderable.hpp"
#include "components/Terrain.hpp"
#include "components/Transform.hpp"
#include "components/Camera.hpp"
#include "components/DebugLine.hpp"

#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/AnimationSystem.hpp"

namespace tr::gp {
   EntitySystem::EntitySystem() {
      registry = std::make_unique<entt::registry>();
   }

   EntitySystem::~EntitySystem() {
   }

   void EntitySystem::fixedUpdate([[maybe_unused]] const cm::Timer& timer,
                                  const AnimationFactory& animationFactory) {
      std::unique_lock lock{registryMutex};
      sys::CameraSystem::fixedUpdate(*registry);
      sys::TransformSystem::update(*registry);
      sys::AnimationSystem::update(*registry, animationFactory);
   }

   void EntitySystem::prepareRenderData(cm::gpu::RenderData& renderData) {
      auto lock = std::shared_lock{registryMutex};
      sys::RenderDataSystem::update(*registry, renderData);
   }

   void EntitySystem::writeCameras(const std::function<void(entt::entity, cmp::Camera)>& fn) {
      auto lock = std::unique_lock{registryMutex};
      registry->view<cmp::Camera>().each(fn);
   }

   void EntitySystem::writeCameras(
       std::function<void(entt::entity, cmp::Camera, uint32_t width, uint32_t height)> fn) {

      auto lock = std::unique_lock{registryMutex};
      const auto [width, height] = registry->ctx().get<const cmp::WindowDimensions>();
      registry->view<cmp::Camera>().each(
          [&width, &height, &fn](auto entity, auto cam) { fn(entity, cam, width, height); });
   }

   void EntitySystem::writeWindowDimensions(const std::pair<uint32_t, uint32_t> size) {
      auto lock = std::unique_lock{registryMutex};
      registry->ctx().insert_or_assign<cmp::WindowDimensions>(
          cmp::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

   auto EntitySystem::createTerrain(const cm::ModelData& handles) -> cm::EntityType {
      ZoneNamedN(n, "entitySystem.createTerrain", true);
      auto lock = std::unique_lock{registryMutex};

      const auto e = registry->create();

      registry->emplace<cmp::Renderable>(e, std::vector{handles.meshData});
      registry->emplace<cmp::TerrainMarker>(e);
      registry->emplace<cmp::Transform>(e, glm::zero<glm::vec3>(), glm::vec3(0.f, -5.f, -50.f));

      const auto debugConstants = registry->create();
      registry->emplace<cmp::Transform>(debugConstants,
                                        glm::zero<glm::vec3>(),
                                        glm::vec3(200.f, 1000.f, 200.f));
      registry->emplace<cmp::DebugConstants>(debugConstants, 16.f);
      return e;
   }

   auto EntitySystem::createStaticModel(const cm::ModelData& handles) -> cm::EntityType {
      auto lock = std::unique_lock{registryMutex};
      const auto e = registry->create();
      registry->emplace<cmp::Renderable>(e, std::vector{handles.meshData});
      registry->emplace<cmp::Transform>(e);
      return e;
   }

   auto EntitySystem::createAnimatedModel(const cm::ModelData& modelData) -> cm::EntityType {
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

   auto EntitySystem::createCamera(uint32_t width,
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

   void EntitySystem::setCurrentCamera(const cm::EntityType currentCamera) {
      auto lock = std::unique_lock{registryMutex};
      registry->ctx().insert_or_assign<cmp::CurrentCamera>(cmp::CurrentCamera{currentCamera});
   }

   [[nodiscard]] auto EntitySystem::createDebugAABB(const glm::vec3& min, const glm::vec3& max)
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

   [[nodiscard]] auto EntitySystem::createDebugTriangle(
       const std::array<glm::vec3, 3> vertices) const -> cm::EntityType {
   }

   [[nodiscard]] auto EntitySystem::createDebugLine(const glm::vec3& start, const glm::vec3& end)
       -> cm::EntityType {
      auto lock = std::unique_lock{registryMutex};
      const auto lineEntity = registry->create();
      registry->emplace<cmp::DebugLine>(lineEntity, start, end);
      return lineEntity;
   }

   [[nodiscard]] auto EntitySystem::createDebugPoint(const glm::vec3& position) const
       -> cm::EntityType {
   }

   void EntitySystem::removeAll() {
      auto lock = std::unique_lock{registryMutex};
      registry->clear();
      registry->ctx().erase<cmp::CurrentCamera>();
   }

}