#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
struct Transform {
   explicit Transform() :
       position(glm::one<glm::vec3>()), rotation(glm::identity<glm::quat>()),
       scale(glm::one<glm::vec3>()) {
   }
   glm::vec3 position;
   glm::quat rotation;
   glm::vec3 scale;

   [[nodiscard]] glm::mat4 getTransform() {
      auto transform = glm::identity<glm::mat4>();
      transform = glm::translate(transform, position);
      transform *= glm::toMat4(rotation);
      transform = glm::scale(transform, scale);
      return transform;
   }

   void rotateByAxisAngle(glm::vec3 axis, float angle) {
      glm::quat newRot = glm::angleAxis(glm::radians(angle), axis);
      rotation *= newRot;
   }

   [[nodiscard]] glm::vec3 getPosition() const {
      return position;
   }
};
