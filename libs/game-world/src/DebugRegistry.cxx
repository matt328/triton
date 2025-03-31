#include "DebugRegistry.hpp"
#include "bk/DebugShapes.hpp"

namespace tr {

DebugTools::DebugTools(std::shared_ptr<IResourceProxy> newResourceProxy)
    : resourceProxy{std::move(newResourceProxy)} {
  Log.trace("Creating DebugRegistry");
}

DebugTools::~DebugTools() {
  Log.trace("Destroying DebugRegistry");
}

auto DebugTools::addDebugCube(std::string tag, glm::vec3 center, float extent, Color color)
    -> void {
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
    lines.emplace_back(corners[pair[0]], corners[pair[1]], color);
  }

  shapeMap.emplace(tag, std::make_unique<Cube>(lines, color));
  dirty = true;
}

auto DebugTools::clearTag(const std::string& tag) -> void {
  shapeMap.erase(tag);
}

auto DebugTools::clearAll() -> void {
  shapeMap.clear();
}

auto DebugTools::getShapes() const -> std::vector<std::shared_ptr<IShape>> {
  auto shapes = std::vector<std::shared_ptr<IShape>>{};
  for (const auto& [key, value] : shapeMap) {
    shapes.push_back(value);
  }
  return shapes;
};

auto DebugTools::setDirty(bool newDirty) -> void {
  dirty = newDirty;
}

auto DebugTools::isDirty() const -> bool {
  return dirty;
}

}
