#pragma once

namespace tr::gfx::geo {
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
}
