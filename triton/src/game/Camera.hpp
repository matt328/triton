#pragma once

struct Camera {
   glm::mat4 projectionMatrix = glm::identity<glm::mat4>();
   glm::mat4 viewMatrix = glm::identity<glm::mat4>();

   float pitch = 0.f;
   float yaw = 0.f;

   bool isActive = true;
};
