#pragma once

namespace tr {

struct AABB {
  glm::vec3 min;
  glm::vec3 max;

  // Check if a point is inside the box
  [[nodiscard]] auto contains(const glm::vec3& point) const -> bool {
    return (point.x >= min.x && point.x <= max.x) && (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
  }

  // Check if another AABB intersects with this one
  [[nodiscard]] auto intersects(const AABB& other) const -> bool {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
  }
};

}
