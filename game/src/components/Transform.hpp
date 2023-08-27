#pragma once

struct Transform {
   glm::vec3 position = glm::zero<glm::vec3>();
   glm::quat rotation = glm::identity<glm::quat>();
   glm::vec3 velocity = glm::zero<glm::vec3>();

   [[nodiscard]] glm::mat4 getTransform() {
      auto transform = glm::identity<glm::mat4>();
      transform = glm::translate(transform, position);
      transform *= glm::toMat4(rotation);
      return transform;
   }

   void rotateByAxisAngle(glm::vec3 axis, float angle) {
      glm::quat newRot = glm::angleAxis(glm::radians(angle), axis);
      rotation *= newRot;
   }

   void setVelocity(glm::vec3 newVelocity) {
      velocity = newVelocity;
   }

   [[nodiscard]] glm::vec3 getPosition() const {
      return position;
   }
};
