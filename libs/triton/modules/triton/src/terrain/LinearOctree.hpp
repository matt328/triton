#pragma once

#include "OctreeNode.hpp"

namespace tr {

constexpr uint8_t MaxChildren = 8;

class LinearOctree {
public:
  LinearOctree(glm::ivec3 newRootPosition, uint32_t newRootSize, uint32_t newRootDepth);
  ~LinearOctree() = default;

  LinearOctree(const LinearOctree&) = delete;
  LinearOctree(LinearOctree&&) = delete;
  auto operator=(const LinearOctree&) -> LinearOctree& = delete;
  auto operator=(LinearOctree&&) -> LinearOctree& = delete;

  [[nodiscard]] auto getRootNode() const -> const OctreeNode&;
  [[nodiscard]] auto nodeHasChildren(const OctreeNode& node) const -> bool;
  [[nodiscard]] auto getChild(const OctreeNode& node, uint8_t index) const -> const OctreeNode&;
  [[nodiscard]] auto getNodeAt(glm::ivec3 position) const -> const OctreeNode&;
  auto removeNode(size_t locCode) -> void;
  auto splitNode(const OctreeNode& node) -> void;

  auto debug() const -> void;
  auto debugNode(const OctreeNode& node) const -> void;

private:
  std::unordered_map<size_t, OctreeNode> nodeMap;
  glm::ivec3 rootPosition;
  uint32_t rootSize;
  uint32_t rootDepth;
  uint32_t leafSize;

  auto createRootNode() -> void;
};

}
