#pragma once

#include "as/Vertex.hpp"

namespace tr::gfx::geo {

   static constexpr auto CubeVerts = std::array<as::Vertex, 24>{
       {{{0.f, 0.f, 0.f}}, {{1.f, 0.f, 0.f}}, {{0.f, 0.f, 0.f}}, {{0.f, 0.f, 1.f}},
        {{0.f, 0.f, 0.f}}, {{0.f, 1.f, 0.f}}, {{1.f, 0.f, 0.f}}, {{1.f, 0.f, 1.f}},
        {{1.f, 0.f, 0.f}}, {{1.f, 1.f, 0.f}}, {{0.f, 0.f, 1.f}}, {{0.f, 1.f, 1.f}},
        {{0.f, 0.f, 1.f}}, {{1.f, 0.f, 1.f}}, {{1.f, 0.f, 1.f}}, {{1.f, 1.f, 1.f}},
        {{0.f, 1.f, 0.f}}, {{0.f, 1.f, 1.f}}, {{0.f, 1.f, 0.f}}, {{1.f, 1.f, 0.f}},
        {{1.f, 1.f, 0.f}}, {{1.f, 1.f, 1.f}}, {{0.f, 1.f, 1.f}}, {{1.f, 1.f, 1.f}}}};

   inline auto transformUnitTriangleToPoints(const glm::vec3& point0,
                                             const glm::vec3& point1,
                                             const glm::vec3& point2) -> glm::mat4 {
      // Compute edge vectors for the new triangle
      glm::vec3 edge1 = point1 - point0;
      glm::vec3 edge2 = point2 - point0;

      // Compute the normalized X, Y, and Z axes for the new triangle basis
      glm::vec3 xAxis = glm::normalize(edge1);
      glm::vec3 zAxis = glm::normalize(glm::cross(edge1, edge2));
      glm::vec3 yAxis = glm::cross(zAxis, xAxis);

      // Scaling factors: length of edges of the triangle
      float scaleX = glm::length(edge1);
      float scaleY = glm::length(glm::cross(edge1, edge2)) / scaleX;

      // Construct the rotation-scaling matrix
      auto rotationScalingMatrix = glm::mat4(1.0f);
      rotationScalingMatrix[0] = glm::vec4(xAxis * scaleX, 0.0f);
      rotationScalingMatrix[1] = glm::vec4(yAxis * scaleY, 0.0f);
      rotationScalingMatrix[2] = glm::vec4(zAxis, 0.0f);

      // Translation to move the triangle to P0
      glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), point0);

      // Combine translation and rotation-scaling to form the final transformation matrix
      glm::mat4 transform = translationMatrix * rotationScalingMatrix;

      return transform;
   }

   inline auto computeAABBTransform(const glm::vec3& min, const glm::vec3& max) -> glm::mat4 {
      glm::vec3 scaleFactors = max - min;
      glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleFactors);
      glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), min);
      glm::mat4 transformMatrix = translationMatrix * scaleMatrix;
      return transformMatrix;
   }
}
