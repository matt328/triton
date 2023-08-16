#pragma once

class CameraSystem {
 public:
   CameraSystem();
   ~CameraSystem();

   CameraSystem(const CameraSystem&) = default;
   CameraSystem(CameraSystem&&) = delete;
   CameraSystem& operator=(const CameraSystem&) = default;
   CameraSystem& operator=(CameraSystem&&) = delete;

   void update(entt::registry& registry);
};
