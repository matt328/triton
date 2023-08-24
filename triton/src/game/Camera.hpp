#pragma once

struct Camera {
   explicit Camera(float fov, float width, float height, float zNear, float zFar, glm::vec3 eye) :
       projectionMatrix(glm::perspective(fov, width / height, zNear, zFar)),
       viewMatrix(glm::lookAt(eye, glm::zero<glm::vec3>(), glm::vec3(0.f, -1.f, 0.f))) {
   }

   glm::mat4 projectionMatrix = glm::identity<glm::mat4>();
   glm::mat4 viewMatrix = glm::identity<glm::mat4>();

   float pitch = 0.f;
   float yaw = 0.f;
};
