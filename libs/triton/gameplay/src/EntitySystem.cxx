#include "EntitySystem.hpp"

#include <entt/entity/fwd.hpp>

#include "cm/Handles.hpp"

#include "components/Animation.hpp"
#include "components/Resources.hpp"
#include "components/DebugConstants.hpp"
#include "components/Renderable.hpp"
#include "components/Terrain.hpp"
#include "components/Transform.hpp"
#include "components/Camera.hpp"
#include "components/Resources.hpp"
#include "components/DebugConstants.hpp"

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
      std::unique_lock<std::shared_mutex> lock{registryMutex};
      sys::CameraSystem::fixedUpdate(*registry);
      sys::TransformSystem::update(*registry);
      sys::AnimationSystem::update(*registry, animationFactory);
   }

   void EntitySystem::prepareRenderData(cm::RenderData& renderData) {
      auto lock = std::shared_lock<std::shared_mutex>{registryMutex};
      sys::RenderDataSystem::update(*registry, renderData);
   }

   void EntitySystem::writeCameras(std::function<void(entt::entity, cmp::Camera)> fn) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->view<cmp::Camera>().each(fn);
   }

   void EntitySystem::writeCameras(
       std::function<void(entt::entity, cmp::Camera, uint32_t width, uint32_t height)> fn) {

      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      const auto [width, height] = registry->ctx().get<const cmp::WindowDimensions>();
      registry->view<cmp::Camera>().each(
          [&width, &height, &fn](auto entity, auto cam) { fn(entity, cam, width, height); });
   }

   void EntitySystem::writeWindowDimensions(const std::pair<uint32_t, uint32_t> size) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->ctx().insert_or_assign<cmp::WindowDimensions>(
          cmp::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

   auto EntitySystem::createTerrain(const cm::MeshHandles handles) -> cm::EntityType {
      ZoneNamedN(n, "entitySystem.createTerrain", true);
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};

      auto e = registry->create();

      registry->emplace<cmp::Renderable>(e, handles);
      registry->emplace<cmp::TerrainMarker>(e);
      registry->emplace<gp::cmp::Transform>(e,
                                            glm::zero<glm::vec3>(),
                                            glm::vec3(-550.f, -1000.f, -5700.f));

      auto debugConstants = registry->create();
      registry->emplace<gp::cmp::Transform>(debugConstants,
                                            glm::zero<glm::vec3>(),
                                            glm::vec3(200.f, 1000.f, 200.f));
      registry->emplace<cmp::DebugConstants>(debugConstants, 16.f);
      return e;
   }

   auto EntitySystem::createStaticModel(const cm::MeshHandles handles) -> cm::EntityType {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};

      auto e = registry->create();
      registry->emplace<cmp::Renderable>(e, handles);
      registry->emplace<gp::cmp::Transform>(e);
      return e;
   }

   auto EntitySystem::createAnimatedModel(const cm::LoadedSkinnedModelData modelData)
       -> cm::EntityType {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      auto e = registry->create();

      registry->emplace<cmp::Animation>(e,
                                        modelData.animationHandle,
                                        modelData.skeletonHandle,
                                        modelData.jointMap,
                                        modelData.inverseBindMatrices);
      registry->emplace<gp::cmp::Transform>(e);

      const auto meshes = std::unordered_map<cm::MeshHandle, cm::TextureHandle>{
          {modelData.meshHandle, modelData.textureHandle}};

      registry->emplace<cmp::Renderable>(e, meshes);

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
      registry->emplace<gp::cmp::Camera>(camera, width, height, fov, zNear, zFar, position);
      return camera;
   }

   void EntitySystem::setCurrentCamera(cm::EntityType currentCamera) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->ctx().insert_or_assign<cmp::CurrentCamera>(cmp::CurrentCamera{currentCamera});
   }

   void EntitySystem::removeAll() {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};
      registry->clear();
      registry->ctx().erase<cmp::CurrentCamera>();
   }

}