#pragma once

namespace tr {

enum class ShapeType : uint8_t {
  Plane = 0,
  Box
};

struct SdfCreateInfo {
  ShapeType shapeType = ShapeType::Plane;
  float voxelSize{};
  float height{};
  glm::vec3 center;
  float size;
};

}
