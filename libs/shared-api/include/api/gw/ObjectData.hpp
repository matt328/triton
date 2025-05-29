#pragma once

#include "bk/Handle.hpp"

namespace tr {

struct GeometryRegion;
struct MaterialRegion;
struct AnimationRegion;

struct ObjectData {
  size_t transformIndex;
  size_t rotationIndex;
  size_t scaleIndex;

  Handle<GeometryRegion> geometryRegion;
  std::optional<Handle<MaterialRegion>> materialRegion = std::nullopt;
  std::optional<Handle<AnimationRegion>> animationRegion = std::nullopt;
};

}
