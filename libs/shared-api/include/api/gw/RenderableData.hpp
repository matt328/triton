#pragma once

#include "api/gfx/DDGeometryData.hpp"
#include "api/gfx/RenderStyle.hpp"
#include "as/Model.hpp"

namespace tr {

using SkeletonHandle = uint64_t;
using AnimationHandle = uint64_t;

struct AnimationData2 {
  SkeletonHandle skeletonHandle{};
  AnimationHandle animationHandle{};
};

struct ObjectData {
  glm::mat4 modelMatrix = glm::identity<glm::mat4>();
  RenderPassType renderPassType;
};

struct MaterialData {
  std::optional<glm::vec4> albedo = std::nullopt;
  std::optional<as::ImageData> imageData = std::nullopt;
};

/// Asset Service will be updated to produce these. This struct will probably be extended with
/// abstract attributes that describe how to render it.
struct RenderableData {
  DDGeometryData geometryData;
  MaterialData materialData;
  ObjectData initialObjectData{};
  std::optional<AnimationData2> animationData = std::nullopt;
};

}
