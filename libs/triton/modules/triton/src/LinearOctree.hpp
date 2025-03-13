#pragma once

namespace tr {

struct OctreeNode;

class LinearOctree {
public:
  LinearOctree(glm::ivec3 newRootPosition, uint32_t newRootSize, uint32_t newRootDepth);
  ~LinearOctree() = default;

  LinearOctree(const LinearOctree&) = delete;
  LinearOctree(LinearOctree&&) = delete;
  auto operator=(const LinearOctree&) -> LinearOctree& = delete;
  auto operator=(LinearOctree&&) -> LinearOctree& = delete;

  auto getRootNode() -> OctreeNode&;
  auto nodeHasChildren(const OctreeNode& node) -> bool;
  auto getChild(const OctreeNode& node, uint8_t index) -> OctreeNode&;
  auto getNodeAt(glm::ivec3 position) -> OctreeNode&;
  auto removeNode(size_t locCode) -> void;
  auto splitNode(const OctreeNode& node) -> void;

private:
  std::unordered_map<size_t, OctreeNode> nodeMap;
  glm::ivec3 rootPosition;
  uint32_t rootSize;
  uint32_t rootDepth;
  uint32_t leafSize;

  auto createRootNode() -> void;
};

}
