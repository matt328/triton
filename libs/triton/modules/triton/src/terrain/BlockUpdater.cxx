#include "BlockUpdater.hpp"
#include "AABB.hpp"

namespace tr {

BlockUpdater::BlockUpdater(std::shared_ptr<LinearOctree> newOctree, glm::vec3 targetPosition)
    : octree{std::move(newOctree)}, targetPosition{targetPosition} {
}

auto BlockUpdater::execute() -> void {
  getChunkUpdates(octree->getRootNode());
  buildTransitionMasks();
}

auto BlockUpdater::buildTransitionMasks() -> void {
  const auto worldBox = AABB{.min = octree->getRootNode().position,
                             .max = glm::vec3{octree->getRootNode().extents,
                                              octree->getRootNode().extents,
                                              octree->getRootNode().extents}};
  for (auto& update : blockUpdates) {
    if (update.updateType == BlockUpdateType::Remove) {
      continue;
    }

    auto neighborsMask = 0u;
    auto halfLeafSize = octree->getLeafSize() >> 1u;
    const auto scaledLeafSize = static_cast<int32_t>((halfLeafSize << update.lod) + 5);

    constexpr std::array<uint8_t, 6> oppositeMask =
        {0b000010, 0b000001, 0b100000, 0b010000, 0b001000, 0b000100};

    for (size_t j = 0; j < std::size(TransitionDirections); ++j) {
      auto position = update.position + TransitionDirections.at(j) * scaledLeafSize;

      if (!worldBox.contains(position)) {
        continue;
      }

      auto otherNode = octree->getNodeAt(position);

      const auto neighborBit = 1u << j;
      const auto neighborBitOpposite = oppositeMask.at(j);

      if (otherNode.depth < update.lod) {
        neighborsMask |= neighborBit;
      }

      const auto desiredOtherNeighborBit = otherNode.depth > update.lod ? neighborBitOpposite : 0u;

      auto it = activeNodesNeighbors.find(otherNode.position);
      auto otherNeighborsMask = it != activeNodesNeighbors.end() ? it->second : 0u;
      auto isolatedOtherNeighborBit = otherNeighborsMask & neighborBitOpposite;

      bool lodMismatch = isolatedOtherNeighborBit != desiredOtherNeighborBit;

      if (lodMismatch) {
        otherNeighborsMask ^= neighborBitOpposite;

        if (!blockUpdatesMap.contains(otherNode.position)) {
          blockUpdates.emplace_back(BlockUpdate{.updateType = BlockUpdateType::Update,
                                                .position = otherNode.position,
                                                .lod = static_cast<uint8_t>(otherNode.depth),
                                                .neighborsMask = otherNeighborsMask});
          blockUpdatesMap.emplace(otherNode.position, blockUpdates.size() - 1);
        } else {
          auto& otherNodeUpdate = blockUpdates[blockUpdatesMap[otherNode.position]];
          otherNodeUpdate.neighborsMask = otherNeighborsMask;
        }
        activeNodesNeighbors[otherNode.position] = otherNeighborsMask;
        update.neighborsMask = neighborsMask;
      }
    }

    activeNodesNeighbors[update.position] = neighborsMask;
  }
}

auto BlockUpdater::getChunkUpdates(const OctreeNode& fromNode) -> void {
  if (canRender(fromNode)) {
    if (!activeNodes.contains(fromNode.locCode)) {
      auto blockUpdate = BlockUpdate{.updateType = BlockUpdateType::Create,
                                     .position = fromNode.position,
                                     .lod = static_cast<uint8_t>(fromNode.depth),
                                     .neighborsMask = 0};
      if (octree->nodeHasChildren(fromNode)) {
        addMergedLeavesUpdates(fromNode);
      }
      blockUpdates.push_back(blockUpdate);
      blockUpdatesMap.emplace(blockUpdate.position, blockUpdates.size() - 1);
      activeNodes.insert(fromNode.locCode);
      activeNodesNeighbors.emplace(blockUpdate.position, 0);
    }
  } else {
    if (activeNodes.contains(fromNode.locCode)) {
      auto blockUpdate = BlockUpdate{.updateType = BlockUpdateType::Remove,
                                     .position = fromNode.position,
                                     .lod = static_cast<uint8_t>(fromNode.depth),
                                     .neighborsMask = 0};
      blockUpdates.push_back(blockUpdate);
      activeNodes.erase(fromNode.locCode);
      activeNodesNeighbors.erase(fromNode.position);
    }
    if (!octree->nodeHasChildren(fromNode)) {
      octree->splitNode(fromNode);
    }
    for (uint8_t i = 0; i < 8; ++i) {
      getChunkUpdates(octree->getChild(fromNode, i));
    }
  }
}

auto BlockUpdater::addMergedLeavesUpdates(const OctreeNode& fromNode) -> void {
  for (auto i = 0u; i < 8u; ++i) {
    auto child = octree->getChild(fromNode, i);
    if (octree->nodeHasChildren(child)) {
      addMergedLeavesUpdates(child);
    } else {
      blockUpdates.emplace_back(BlockUpdate{.updateType = BlockUpdateType::Update,
                                            .position = child.position,
                                            .lod = static_cast<uint8_t>(child.depth),
                                            .neighborsMask = 0});
      activeNodes.erase(child.locCode);
      activeNodesNeighbors.erase(child.position);
    }

    octree->removeNode(child.locCode);
  }
}

auto BlockUpdater::canRender(const OctreeNode& node) -> bool {
  if (node.depth == 0) {
    return true;
  }

  auto distx = std::abs(node.position.x - targetPosition.x);
  auto disty = std::abs(node.position.y - targetPosition.y);
  auto distz = std::abs(node.position.z - targetPosition.z);

  auto minDist = std::max({distx, disty, distz});

  auto compareDist = octree->getLeafSize() * 1.5f * (1 << node.depth);

  return minDist > compareDist;
}

}
