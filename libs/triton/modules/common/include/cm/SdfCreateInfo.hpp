#pragma once

namespace tr {

enum class ShapeType : uint8_t {
  Plane = 0,
  Box,
  Sphere
};

struct PlaneInfo {
  float height{};
  glm::vec3 normal;
};

struct BoxInfo {
  glm::vec3 center;
  float size;
};

struct SphereInfo {
  glm::vec3 center;
  float radius;
};

struct SdfCreateInfo {
  ShapeType shapeType = ShapeType::Plane;
  std::variant<PlaneInfo, BoxInfo, SphereInfo> shapeInfo;

  SdfCreateInfo() : shapeInfo(PlaneInfo{}) {
  }

  template <typename T>
  auto get() -> T& {
    return std::get<T>(shapeInfo);
  }
};
}
