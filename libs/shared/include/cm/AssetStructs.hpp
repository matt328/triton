#pragma once

#include "cm/Handles.hpp"

namespace tr {

using AnimationHandle = uint64_t;

enum class Topology : uint8_t {
  Triangles = 0,
  LineList = 1
};

/// Combination of MeshHandle, Topology, and a TextureHandle
struct MeshData {
  MeshHandle meshHandle{};
  Topology topology{Topology::Triangles};
  TextureHandle textureHandle{};
};

struct SkinData {
  std::map<int, int> jointMap;
  std::vector<glm::mat4> inverseBindMatrices;
};

struct AnimationData {
  SkeletonHandle skeletonHandle{};
  AnimationHandle animationHandle{};
};

/// Mesh and Optional Skin and Animation Data
struct ModelData {
  MeshData meshData{};
  std::optional<SkinData> skinData;
  std::optional<AnimationData> animationData;
};

}
