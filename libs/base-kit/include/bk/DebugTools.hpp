#pragma once

#include "DebugShapes.hpp"

namespace tr {

struct ShapeEntry {
  std::shared_ptr<IShape> shape;
  float duration;
};

class DebugTools {
public:
  static auto instance() -> DebugTools& {
    static DebugTools instance;
    return instance;
  }

  DebugTools(DebugTools&&) = delete;
  auto operator=(DebugTools&&) -> DebugTools& = delete;
  DebugTools(const DebugTools&) = delete;
  auto operator=(const DebugTools&) -> DebugTools& = delete;

  auto drawCube(glm::vec3 center, float extent, Color color, float duration = 0.f) -> void {
    std::array<glm::vec3, 8> corners = {
        center + glm::vec3(-extent, -extent, -extent), // 0
        center + glm::vec3(extent, -extent, -extent),  // 1
        center + glm::vec3(extent, extent, -extent),   // 2
        center + glm::vec3(-extent, extent, -extent),  // 3
        center + glm::vec3(-extent, -extent, extent),  // 4
        center + glm::vec3(extent, -extent, extent),   // 5
        center + glm::vec3(extent, extent, extent),    // 6
        center + glm::vec3(-extent, extent, extent)    // 7
    };

    int edgePairs[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0}, // Front face
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4}, // Back face
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7} // Side edges
    };

    std::vector<Line> lines;
    lines.reserve(12);
    for (auto* pair : edgePairs) {
      lines.emplace_back(corners[pair[0]], corners[pair[1]], color, duration);
    }

    shapeEntries.emplace_back(std::make_unique<Cube>(lines, color, duration));
  }

  [[nodiscard]] auto getShapes() const -> std::vector<std::shared_ptr<IShape>> {
    auto newShapes = std::vector<std::shared_ptr<IShape>>{};
    newShapes.reserve(shapeEntries.size());
    for (const auto& entry : shapeEntries) {
      newShapes.push_back(entry.shape);
    }
    return newShapes;
  };

private:
  DebugTools() = default;
  ~DebugTools() = default;

  std::vector<ShapeEntry> shapeEntries;
};

}
