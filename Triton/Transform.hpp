#pragma once

struct Transform {
   explicit Transform(const glm::mat4 transform) : transform(transform) {
   }
   glm::mat4 transform;

   explicit operator glm::mat4() const {
      return transform;
   }

   glm::vec3 getPosition() const {
      glm::vec3 scale;
      glm::quat rotation;
      glm::vec3 translation;
      glm::vec3 skew;
      glm::vec4 perspective;
      decompose(transform, scale, rotation, translation, skew, perspective);
      return translation;
   }
};
