#pragma once

#include "api/gfx/DDGeometryData.hpp"
#include "as/Model.hpp"
#include "dd/RenderConfig.hpp"

namespace tr {

using SkeletonHandle = uint64_t;
using AnimationHandle = uint64_t;

struct AnimationData2 {
  SkeletonHandle skeletonHandle{};
  AnimationHandle animationHandle{};
};

struct ObjectData {
  std::vector<glm::mat4> modelMatrix;
};

struct MaterialData {
  ShadingMode shadingMode = ShadingMode::Wireframe;
  std::optional<glm::vec4> albedo = std::nullopt;
  std::optional<as::ImageData> imageData = std::nullopt;
};

/// Asset Service will be updated to produce these. This struct will probably be extended with
/// abstract attributes that describe how to render it.
struct RenderableData {
  DDGeometryData geometryData;
  MaterialData materialData;
  ObjectData objectData;
  std::optional<AnimationData2> animationData = std::nullopt;
};

}
