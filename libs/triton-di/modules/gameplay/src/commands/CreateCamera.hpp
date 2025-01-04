#pragma once

#include "CommandQueue.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/EditorInfo.hpp"
#include "gp/components/Resources.hpp"

namespace tr {

class VkResourceManager;

class CreateCamera final
    : public ICommand<entt::registry&, const std::shared_ptr<VkResourceManager>&> {
public:
  explicit CreateCamera(const CameraInfo& newCameraInfo) : cameraInfo{newCameraInfo} {};
  ~CreateCamera() override = default;

  CreateCamera(const CreateCamera& other) = delete;
  CreateCamera(CreateCamera&& other) noexcept = delete;
  auto operator=(const CreateCamera& other) -> CreateCamera& = delete;
  auto operator=(CreateCamera&& other) noexcept -> CreateCamera& = delete;

  void execute(
      entt::registry& registry,
      [[maybe_unused]] const std::shared_ptr<VkResourceManager>& resourceManager) const override {
    const auto entity = registry.create();
    registry.emplace<Camera>(entity,
                             cameraInfo.width,
                             cameraInfo.height,
                             cameraInfo.fov,
                             cameraInfo.nearClip,
                             cameraInfo.farClip,
                             cameraInfo.position);
    registry.emplace<EditorInfo>(entity, "Default Camera");
    registry.ctx().insert_or_assign<CurrentCamera>(CurrentCamera{entity});
  }

private:
  CameraInfo cameraInfo{};
};

}
