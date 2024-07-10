#include "EntitySystem.hpp"

#include <entt/entity/fwd.hpp>

#include "cm/Handles.hpp"

#include "cmp/Animation.hpp"
#include "cmp/Resources.hpp"
#include "cmp/DebugConstants.hpp"
#include "cmp/Renderable.hpp"
#include "cmp/Terrain.hpp"
#include "cmp/Transform.hpp"
#include "cmp/Camera.hpp"
#include "cmp/Resources.hpp"
#include "cmp/DebugConstants.hpp"

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
                                  AnimationFactory& animationFactory) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};

      ecs::CameraSystem::fixedUpdate(*registry);
      ecs::TransformSystem::update(*registry);
      ecs::AnimationSystem::update(*registry, animationFactory);
   }

   void EntitySystem::prepareRenderData(cm::RenderData& renderData) {
      auto lock = std::shared_lock<std::shared_mutex>{registryMutex};

      ecs::RenderDataSystem::update(*registry, renderData);
   }

   void EntitySystem::writeCameras(std::function<void(entt::entity, ecs::Camera)> fn) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->view<ecs::Camera>().each(fn);
   }

   void EntitySystem::writeCameras(
       std::function<void(entt::entity, ecs::Camera, uint32_t width, uint32_t height)> fn) {

      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      const auto [width, height] = registry->ctx().get<const ecs::WindowDimensions>();
      registry->view<ecs::Camera>().each(
          [&width, &height, &fn](auto entity, auto cam) { fn(entity, cam, width, height); });
   }

   void EntitySystem::writeWindowDimensions(const std::pair<uint32_t, uint32_t> size) {
      auto lock = std::shared_lock<std::shared_mutex>{registryMutex};
      registry->ctx().insert_or_assign<ecs::WindowDimensions>(
          ecs::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

   auto EntitySystem::createTerrain(const cm::MeshHandles handles) -> cm::EntityType {
      auto lock = std::shared_lock<std::shared_mutex>{registryMutex};

      auto e = registry->create();

      registry->emplace<gp::ecs::Renderable>(e, handles);
      registry->emplace<gp::ecs::TerrainMarker>(e);
      registry->emplace<gp::ecs::Transform>(e,
                                            glm::zero<glm::vec3>(),
                                            glm::vec3(-550.f, -1000.f, -5700.f));

      auto debugConstants = registry->create();
      registry->emplace<gp::ecs::Transform>(debugConstants,
                                            glm::zero<glm::vec3>(),
                                            glm::vec3(200.f, 1000.f, 200.f));
      registry->emplace<gp::ecs::DebugConstants>(debugConstants, 16.f);
      return e;
   }

   auto EntitySystem::createStaticModel(const cm::MeshHandles handles) -> cm::EntityType {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};

      auto e = registry->create();
      registry->emplace<gp::ecs::Renderable>(e, handles);
      registry->emplace<gp::ecs::Transform>(e);
      return e;
   }

   auto EntitySystem::createAnimatedModel(const cm::LoadedSkinnedModelData modelData)
       -> cm::EntityType {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      auto e = registry->create();

      registry->emplace<gp::ecs::Animation>(e,
                                            modelData.animationHandle,
                                            modelData.skeletonHandle,
                                            modelData.jointMap,
                                            modelData.inverseBindMatrices);
      registry->emplace<gp::ecs::Transform>(e);

      const auto meshes = std::unordered_map<cm::MeshHandle, cm::TextureHandle>{
          {modelData.meshHandle, modelData.textureHandle}};

      registry->emplace<gp::ecs::Renderable>(e, meshes);

      return e;
   }

   auto EntitySystem::createCamera(uint32_t width,
                                   uint32_t height,
                                   float fov,
                                   float zNear,
                                   float zFar,
                                   glm::vec3 position,
                                   [[maybe_unused]] std::optional<std::string> name)
       -> cm::EntityType {

      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      const auto camera = registry->create();
      registry->emplace<gp::ecs::Camera>(camera, width, height, fov, zNear, zFar, position);
      return camera;
   }

   void EntitySystem::setCurrentCamera(cm::EntityType currentCamera) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->ctx().insert_or_assign<gp::ecs::CurrentCamera>(
          gp::ecs::CurrentCamera{currentCamera});
   }

   void EntitySystem::removeAll() {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->clear();
      registry->ctx().erase<gp::ecs::CurrentCamera>();
   }

}