#include "gp/EntitySystem.hpp"

#include <entt/entity/fwd.hpp>

#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/DebugConstants.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Terrain.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/DebugConstants.hpp"

#include "gp/ecs/system/CameraSystem.hpp"
#include "gp/ecs/system/RenderDataSystem.hpp"
#include "gp/ecs/system/TransformSystem.hpp"
#include "gp/ecs/system/AnimationSystem.hpp"

namespace tr::gp {
   EntitySystem::EntitySystem() {
      registry = std::make_unique<entt::registry>();
   }

   EntitySystem::~EntitySystem() {
   }

   void EntitySystem::fixedUpdate(const util::Timer& timer) {
      auto lock = std::unique_lock<std::shared_mutex>{registryMutex};

      ecs::CameraSystem::fixedUpdate(*registry);
      ecs::TransformSystem::update(*registry);
      ecs::AnimationSystem::update(*registry, animationFactory);
   }

   void EntitySystem::prepareRenderData(gfx::RenderData& renderData) {
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

   auto EntitySystem::createTerrain(const gfx::MeshHandles handles) -> gp::EntityType {
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

}