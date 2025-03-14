#include "LinearOctree.hpp"

namespace tr {

LinearOctree::LinearOctree(glm::ivec3 newRootPosition, uint32_t newRootSize, uint32_t newRootDepth)
    : rootPosition{newRootPosition},
      rootSize{newRootSize},
      rootDepth{newRootDepth},
      leafSize{rootSize >> rootDepth} {
  createRootNode();
}

auto LinearOctree::debug() const -> void {
  Log.debug("LinearOctree State - Root Position: ({}, {}, {}), Root Size: {}, Root Depth: {}, Leaf "
            "Size: {}, Node Count: {}",
            rootPosition.x,
            rootPosition.y,
            rootPosition.z,
            rootSize,
            rootDepth,
            leafSize,
            nodeMap.size());
}

auto LinearOctree::debugNode(const OctreeNode& node) const -> void {
  node.debug();
  if (nodeHasChildren(node)) {
    for (uint8_t i = 0; i < MaxChildren; ++i) {
      const auto& childNode = getChild(node, i);
      debugNode(childNode);
    }
  }
}

auto LinearOctree::createRootNode() -> void {
  nodeMap.emplace(1,
                  OctreeNode{
                      .position = glm::ivec3{},
                      .extents = rootSize / 2,
                      .depth = rootDepth,
                      .locCode = 1,
                  });
}

auto LinearOctree::getRootNode() const -> const OctreeNode& {
  return nodeMap.at(1);
}

auto LinearOctree::nodeHasChildren(const OctreeNode& node) const -> bool {
  return nodeMap.contains(node.locCode << 3u);
}

auto LinearOctree::getChild(const OctreeNode& node, uint8_t index) const -> const OctreeNode& {
  return nodeMap.at(node.locCode << 3u | index);
}

auto LinearOctree::getLeafSize() const -> uint32_t {
  return leafSize;
}

auto LinearOctree::getNodeAt(glm::ivec3 position) const -> const OctreeNode& {
  const auto* currentNode = &getRootNode();
  while (nodeHasChildren(*currentNode)) {
    auto index = ((position.x > currentNode->position.x) ? 1u : 0u) +
                 ((position.y > currentNode->position.y) ? 2u : 0u) +
                 ((position.z > currentNode->position.z) ? 4u : 0u);
    currentNode = &getChild(*currentNode, index);
  }
  return *currentNode;
}

auto LinearOctree::removeNode(size_t locCode) -> void {
  nodeMap.erase(locCode);
}

auto LinearOctree::splitNode(const OctreeNode& node) -> void {
  assert(!nodeMap.contains(node.locCode << 3u));
  assert(node.depth > 0);

  auto childDepth = node.depth - 1;
  auto childExtents = node.extents >> 1u;

  for (uint8_t i = 0; i < MaxChildren; ++i) {
    const auto locCode = (node.locCode << 3u) | i;
    nodeMap.emplace(locCode,
                    OctreeNode{
                        .position = glm::ivec3(childExtents * ((i & 1u) > 0 ? 1 : -1),
                                               childExtents * ((i & 2u) > 0 ? 1 : -1),
                                               childExtents * ((i & 4u) > 0 ? 1 : -1)),
                        .extents = childExtents,
                        .depth = childDepth,
                        .locCode = locCode,
                    });
  }
}

}
