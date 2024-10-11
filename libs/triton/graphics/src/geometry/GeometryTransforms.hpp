#pragma once

namespace tr::gfx::geo {
   inline auto transformUnitTriangleToPoints(const glm::vec3& P0,
                                             const glm::vec3& P1,
                                             const glm::vec3& P2) -> glm::mat4 {
      // Compute edge vectors for the new triangle
      glm::vec3 v1 = P1 - P0; // Edge from P0 to P1
      glm::vec3 v2 = P2 - P0; // Edge from P0 to P2

      // Compute the normalized X, Y, and Z axes for the new triangle basis
      glm::vec3 xAxis = glm::normalize(v1); // X-axis along v1
      glm::vec3 zAxis =
          glm::normalize(glm::cross(v1, v2));     // Z-axis perpendicular to the triangle's plane
      glm::vec3 yAxis = glm::cross(zAxis, xAxis); // Y-axis perpendicular to X and Z axes

      // Scaling factors: length of edges of the triangle
      float scaleX = glm::length(v1);
      float scaleY = glm::length(glm::cross(v1, v2)) / scaleX; // Area-based scaling for Y

      // Construct the rotation-scaling matrix
      auto rotationScalingMatrix = glm::mat4(1.0f);
      rotationScalingMatrix[0] = glm::vec4(xAxis * scaleX, 0.0f); // Scale along X-axis
      rotationScalingMatrix[1] = glm::vec4(yAxis * scaleY, 0.0f); // Scale along Y-axis
      rotationScalingMatrix[2] = glm::vec4(zAxis, 0.0f);          // Z-axis, no scaling needed

      // Translation to move the triangle to P0
      glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), P0);

      // Combine translation and rotation-scaling to form the final transformation matrix
      glm::mat4 transform = translationMatrix * rotationScalingMatrix;

      return transform;
   }
} // namespace tr::gfx::geo
