#pragma once

#include "api/GlmToString.hpp"

namespace tr {

struct OctreeNode {
public:
  /// Center of this node. Can be integer since each node's extents are also integers.
  glm::ivec3 position{};
  /// All nodes are cubes, this is how far it extends from the center. The value is the same in each
  /// direction
  uint32_t extents;
  /// The Depth of the node in the tree
  uint32_t depth;
  /// These indices correspond to a 3-bit value, where each bit represents whether the position is
  /// in the positive or negative half of the node along a given axis.
  size_t locCode;

  auto debug() const -> void {
    std::string indent(depth, ' ');
    Log.debug("{} position={}, extents={}, depth={}, locCode{}",
              indent,
              position,
              extents,
              depth,
              locCode);
  }
};

}
