#pragma once
#include "as/GlmHashes.hpp"
#include "LinearOctree.hpp"

namespace tr {

enum class BlockUpdateType : uint8_t {
  Remove = 0,
  Create,
  Update
};

struct BlockUpdate {
  BlockUpdateType updateType;
  glm::ivec3 position;
  uint8_t lod;
  /* Bit:    5    4    3    2    1    0
     Dir:   +Z   -Z   +Y   -Y   +X   -X
  */
  /// Describes which neighbors of the block in question have a higher LOD, and need to also be
  /// updated
  uint32_t neighborsMask;
};

struct BlockUpdateComparator {
  auto operator()(const BlockUpdate& a, const BlockUpdate& b) const -> bool {
    return a.lod < b.lod;
  }
};

constexpr auto TransitionDirections = std::array<glm::ivec3, 6>{{
    {-1, 0, 0}, // Negative X
    {0, -1, 0}, // Negative Y
    {0, 0, -1}, // Negative Z
    {1, 0, 0},  // Positive X
    {0, 1, 0},  // Positive Y
    {0, 0, 1}   // Positive Z
}};

class BlockUpdater {
public:
  BlockUpdater(std::shared_ptr<LinearOctree> newOctree, glm::vec3 targetPosition);
  ~BlockUpdater() = default;

  BlockUpdater(const BlockUpdater&) = delete;
  BlockUpdater(BlockUpdater&&) = delete;
  auto operator=(const BlockUpdater&) -> BlockUpdater& = delete;
  auto operator=(BlockUpdater&&) -> BlockUpdater& = delete;

  auto execute() -> void;
  auto debug() -> void;

private:
  std::shared_ptr<LinearOctree> octree;
  glm::vec3 targetPosition;
  std::vector<BlockUpdate> blockUpdates;
  std::unordered_set<size_t> activeNodes;
  std::unordered_map<glm::ivec3, uint32_t> activeNodesNeighbors;
  std::unordered_map<glm::ivec3, uint32_t> blockUpdatesMap;

  auto buildTransitionMasks() -> void;
  auto getChunkUpdates(const OctreeNode& fromNode) -> void;
  auto addMergedLeavesUpdates(const OctreeNode& fromNode) -> void;
  auto canRender(const OctreeNode& node) -> bool;
};

}

template <>
struct std::formatter<tr::BlockUpdate> {
  constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tr::BlockUpdate& bu, FormatContext& ctx) const -> FormatContext::iterator {
    return std::format_to(
        ctx.out(),
        "BlockUpdate {{ Type: {}, Position: ({}, {}, {}), LOD: {}, NeighborsMask: {:06b} }}",
        static_cast<int>(bu.updateType),
        bu.position.x,
        bu.position.y,
        bu.position.z,
        bu.lod,
        bu.neighborsMask);
  }
};
