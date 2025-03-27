#pragma once

#include "cm/Handles.hpp"
#include "cm/Topology.hpp"

namespace tr {

// These structs are used by IResourceProxy

using AnimationHandle = uint64_t;

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
/// Basically a Handle to a model with the Animation data since the animation data is updated by
/// gameworld and not in graphics, so that data has to be passed over the gameworld->graphics sync
/// point
struct ModelData {
  MeshData meshData{};
  std::optional<SkinData> skinData;
  std::optional<AnimationData> animationData;
};

}
