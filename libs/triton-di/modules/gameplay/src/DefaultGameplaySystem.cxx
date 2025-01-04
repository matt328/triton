#include "gp/DefaultGameplaySystem.hpp"
#include "commands/CreateStaticEntity.hpp"
#include "gp/components/Resources.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "tr/IEventBus.hpp"
#include "commands/CreateCamera.hpp"
#include <entt/entity/fwd.hpp>
#include <tracy/Tracy.hpp>

namespace tr {

DefaultGameplaySystem::DefaultGameplaySystem(std::shared_ptr<IEventBus> newEventBus,
                                             std::shared_ptr<CameraSystem> newCameraSystem,
                                             std::shared_ptr<VkResourceManager> newResourceManager,
                                             std::shared_ptr<TransformSystem> newTransformSystem,
                                             std::shared_ptr<RenderDataSystem> newRenderDataSystem)
    : eventBus{std::move(newEventBus)},
      cameraSystem{std::move(newCameraSystem)},
      resourceManager{std::move(newResourceManager)},
      transformSystem{std::move(newTransformSystem)},
      renderDataSystem{std::move(newRenderDataSystem)} {

  registry = std::make_unique<entt::registry>();

  entityCreatedConnection =
      registry->on_construct<entt::entity>().connect<&DefaultGameplaySystem::entityCreated>(this);

  commandQueue =
      std::make_unique<CommandQueue<entt::registry&, const std::shared_ptr<VkResourceManager>&>>();

  eventBus->subscribe<SwapchainResized>([&](const SwapchainResized& event) {
    registry->ctx().insert_or_assign<WindowDimensions>(WindowDimensions{event.width, event.height});
  });
}

DefaultGameplaySystem::~DefaultGameplaySystem() {
  Log.trace("Destroying Gameplay System");
  entityCreatedConnection.release();
}

void DefaultGameplaySystem::update() {
  {
    ZoneNamedN(rd, "RenderData", true);
    renderData.objectData.clear();
    renderData.staticMeshData.clear();
    renderData.terrainMeshData.clear();
    renderData.skinnedMeshData.clear();
    renderData.animationData.clear();
    renderDataSystem->update(*registry, renderData);
  }
  transferHandler(renderData);
}

void DefaultGameplaySystem::fixedUpdate() {
  std::unique_lock<LockableBase(std::shared_mutex)> lock(registryMutex);
  LockMark(registryMutex);

  {
    ZoneNamedN(z, "Gameplay Command Queue", true);
    commandQueue->processCommands(*registry, resourceManager);
  }
  {
    ZoneNamedN(camZone, "CameraSystem", true);
    cameraSystem->fixedUpdate(*registry);
  }
  {
    ZoneNamedN(xformZone, "Transform", true);
    transformSystem->update(*registry);
  }
}

void DefaultGameplaySystem::setRenderDataTransferHandler(const RenderDataTransferHandler& handler) {
  this->transferHandler = handler;
}

auto DefaultGameplaySystem::createStaticModelEntity([[maybe_unused]] std::string filename,
                                                    [[maybe_unused]] std::string_view entityName)
    -> void {

  /*
    Think about the api between game world and renderer around loading resources into the renderer.
    I guess for now, the game world can pass filenames to the renderer to have it load them.

    Think about multithreading concerns. I think the registry needs to be synchronized.
    Nothing will ask the renderer to render something until the entity has been created, so long as
    the entity isn't created until after the resource is loaded and the multi buffer update/resize
    is complete, it should be ok.
  */

  commandQueue->enqueue(std::make_unique<CreateStaticEntityCommand>(filename, entityName.data()));
}

auto DefaultGameplaySystem::createAnimatedModelEntity(
    [[maybe_unused]] const AnimatedModelData& modelData) -> void {
  Log.trace("gameplaySystem createAnimatedModelEntity: {0}", modelData);

  // have gfx create the resources and return a handle.

  // use the handle(s) to create an entity in the ECS

  // return static_cast<cm::EntityType>(1);
}

auto DefaultGameplaySystem::createTerrain() -> void {
  Log.trace("Creating terrain");
}

auto DefaultGameplaySystem::createDefaultCamera() -> void {
  const auto [width, height] = registry->ctx().get<const WindowDimensions>();

  auto cameraInfo = CameraInfo{
      .width = width,
      .height = height,
      .fov = 60.f,
      .nearClip = 0.1f,
      .farClip = 10000.f,
      .position = glm::vec3{1.0f, 1.0f, 3.0f},
  };
  commandQueue->enqueue(std::make_unique<CreateCamera>(cameraInfo));
}

auto DefaultGameplaySystem::createTestEntity([[maybe_unused]] std::string_view name) -> void {
  Log.trace("Creating test entity: {}", name.data());
}

auto DefaultGameplaySystem::entityCreated([[maybe_unused]] entt::registry& reg,
                                          [[maybe_unused]] entt::entity entity) const -> void {
  Log.trace("Entity Created: {}", static_cast<uint32_t>(entity));
  eventBus->emit(EntityCreated{entity});
}

}
