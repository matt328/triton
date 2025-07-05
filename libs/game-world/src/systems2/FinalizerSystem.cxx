#include "FinalizerSystem.hpp"
#include "components/Camera.hpp"
#include "components/Renderable.hpp"
#include "components/Resources.hpp"
#include "components/Transform.hpp"
#include "api/GlmToString.hpp"

namespace tr {

auto FinalizerSystem::update(entt::registry& registry, SimState& simState, Timestamp t) -> void {
  ZoneScopedN("FinalizerSystem::update");
  // simState.clear();
  simState = SimState{1};
  simState.timeStamp = t;

  const auto view = registry.view<Renderable, Transform>();
  const auto size = view.size_hint();

  simState.ensureCapacity(size);
  size_t current = 0;

  std::tie(simState.view, simState.projection) = createCameraData(registry);

  for (const auto& [entity, renderable, transform] : view.each()) {
    std::optional<Handle<TextureTag>> textureHandle =
        renderable.textureHandles.empty()
            ? std::nullopt
            : std::make_optional<Handle<TextureTag>>(renderable.textureHandles.front());
    simState.stateHandles.push_back(
        StateHandles{.geometryHandle = renderable.geometryHandles.front(),
                     .textureHandle = textureHandle});

    simState.positions.push_back({.position = transform.position});
    simState.rotations.push_back({.rotation = transform.rotation});
    simState.scales.push_back({.scale = transform.scale});
    simState.objectMetadata.push_back(
        GpuObjectData{.transformIndex = static_cast<uint32_t>(simState.positions.size() - 1),
                      .rotationIndex = static_cast<uint32_t>(simState.rotations.size() - 1),
                      .scaleIndex = static_cast<uint32_t>(simState.scales.size() - 1),
                      .geometryRegionId = static_cast<uint32_t>(current),
                      .materialId = 0L,
                      .animationId = 0L});
    ++current;
  }
}

auto FinalizerSystem::createCameraData(entt::registry& registry)
    -> std::tuple<glm::mat4, glm::mat4> {

  if (!registry.ctx().contains<WindowDimensions>()) {
    return {glm::identity<glm::mat4>(), glm::identity<glm::mat4>()};
  }

  const auto [width, height] = registry.ctx().get<const WindowDimensions>();

  const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
  auto& cam = registry.get<Camera>(cameraEntity.currentCamera);

  auto direction = glm::vec3{cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
                             sin(glm::radians(cam.pitch)),
                             sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch))};

  cam.front = normalize(direction);
  cam.right = normalize(cross(cam.front, glm::vec3(0.0f, 1.0f, 0.0f)));

  glm::mat3 const rotationMatrix{cam.right, worldUp, cam.front};

  const auto rotatedVelocity = rotationMatrix * cam.velocity;

  cam.position += rotatedVelocity;

  cam.view = lookAt(cam.position, cam.position + cam.front, {0.f, 1.f, 0.f});

  const float aspect = static_cast<float>(width) / static_cast<float>(height);
  cam.projection = glm::perspective(glm::radians(cam.fov), aspect, cam.nearClip, cam.farClip);
  // Apparently everyone except me knew glm was for OpenGL and you have to adjust these
  // matrices for Vulkan
  cam.projection[1][1] *= -1;
  return {cam.view, cam.projection};
}

}
