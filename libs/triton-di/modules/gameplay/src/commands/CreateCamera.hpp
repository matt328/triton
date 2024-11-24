#pragma once

#include "../CommandQueue.hpp"

#include <gp/components/Camera.hpp>
#include <gp/components/Resources.hpp>

namespace tr::gp::cmd {
   class CreateCamera final
       : public ICommand<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&> {
    public:
      explicit CreateCamera(const cmp::CameraInfo& newCameraInfo) : cameraInfo{newCameraInfo} {};
      ~CreateCamera() override = default;

      CreateCamera(const CreateCamera& other) = delete;
      CreateCamera(CreateCamera&& other) noexcept = delete;
      auto operator=(const CreateCamera& other) -> CreateCamera& = delete;
      auto operator=(CreateCamera&& other) noexcept -> CreateCamera& = delete;

      void execute(entt::registry& registry,
                   const std::shared_ptr<gfx::ResourceManager>& resourceManager) const override {
         const auto entity = registry.create();
         registry.emplace<cmp::Camera>(entity,
                                       cameraInfo.width,
                                       cameraInfo.height,
                                       cameraInfo.fov,
                                       cameraInfo.nearClip,
                                       cameraInfo.farClip,
                                       cameraInfo.position);
         registry.ctx().insert_or_assign<cmp::CurrentCamera>(cmp::CurrentCamera{entity});
      }

    private:
      cmp::CameraInfo cameraInfo{};
   };
}
