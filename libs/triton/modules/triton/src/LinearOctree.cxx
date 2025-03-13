#include "LinearOctree.hpp"

#include "OctreeNode.hpp"

namespace tr {

LinearOctree::LinearOctree(glm::ivec3 newRootPosition, uint32_t newRootSize, uint32_t newRootDepth)
    : rootPosition{newRootPosition},
      rootSize{newRootSize},
      rootDepth{newRootDepth},
      leafSize{rootSize >> rootDepth} {
  createRootNode();
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

auto LinearOctree::getRootNode() -> OctreeNode& {
  return nodeMap.at(1);
}

auto LinearOctree::nodeHasChildren(const OctreeNode& node) -> bool {
  return nodeMap.contains(node.locCode << 3);
}

auto LinearOctree::getChild(const OctreeNode& node, uint8_t index) -> OctreeNode& {
  return nodeMap.at(node.locCode << 3 | index);
}

auto LinearOctree::getNodeAt(glm::ivec3 position) -> OctreeNode& {
  auto& currentNode = getRootNode();
  while (nodeHasChildren(currentNode)) {
    auto index = ((position.x > currentNode.position.x) ? 1u : 0u) +
                 ((position.y > currentNode.position.y) ? 2u : 0u) +
                 ((position.z > currentNode.position.z) ? 4u : 0u);
    currentNode = getChild(currentNode, index);
  }
  return currentNode;
}

auto LinearOctree::removeNode(size_t locCode) -> void {
  nodeMap.erase(locCode);
}

auto LinearOctree::splitNode(const OctreeNode& node) -> void {
  assert(!nodeMap.contains(node.locCode << 3));
  assert(node.depth > 0);

  auto childDepth = node.depth - 1;
  auto childExtents = node.extents >> 1;

  for (uint8_t i = 0; i < 8; ++i) {
    const auto locCode = (node.locCode << 3) | i;
    nodeMap.emplace(locCode,
                    OctreeNode{
                        .position = glm::ivec3(childExtents * ((i & 1) > 0 ? 1 : -1),
                                               childExtents * ((i & 2) > 0 ? 1 : -1),
                                               childExtents * ((i & 4) > 0 ? 1 : -1)),
                        .extents = childExtents,
                        .depth = childDepth,
                        .locCode = locCode,
                    });
  }
}

}
