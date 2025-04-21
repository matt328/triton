#include "api/fx/GeometryGenerator.hpp"

namespace tr {
auto GeometryGenerator::generateBox(const BoxCreateInfo& info) -> DDGeometryData {
  const auto center = info.center;
  const auto extent = info.extent;
  constexpr size_t CornerCount = 8;
  std::array<glm::vec3, CornerCount> corners = {
      center + glm::vec3(-extent, -extent, -extent), // 0
      center + glm::vec3(extent, -extent, -extent),  // 1
      center + glm::vec3(extent, extent, -extent),   // 2
      center + glm::vec3(-extent, extent, -extent),  // 3
      center + glm::vec3(-extent, -extent, extent),  // 4
      center + glm::vec3(extent, -extent, extent),   // 5
      center + glm::vec3(extent, extent, extent),    // 6
      center + glm::vec3(-extent, extent, extent)    // 7
  };

  // Vertex Format
  constexpr auto positionAttribute = VertexAttribute{.format = VertexAttributeFormat::Float3,
                                                     .stride = sizeof(glm::vec3),
                                                     .semantic = VertexAttributeSemantic::Position};
  constexpr auto colorAttribute = VertexAttribute{.format = VertexAttributeFormat::Float3,
                                                  .stride = sizeof(glm::vec3),
                                                  .semantic = VertexAttributeSemantic::Color};

  auto vertexFormat = VertexFormat{.stride = sizeof(glm::vec3) + sizeof(glm::vec3),
                                   .attributes = std::vector{positionAttribute, colorAttribute}};

  auto vertexList = VertexList{};
  vertexList.reserve(corners.size());
  for (size_t i = 0; i < corners.size(); ++i) {
    // Position
    vertexList.setAttribute(i, positionAttribute, corners.at(i));
    // Color
    vertexList.setAttribute(i, colorAttribute, glm::vec3(info.color.r, info.color.g, info.color.b));
  }

  std::vector<uint32_t> indices = {
      0, 1, 1, 2, 2, 3, 3, 0, // Front face
      4, 5, 5, 6, 6, 7, 7, 4, // Back face
      0, 4, 1, 5, 2, 6, 3, 7  // Side edges
  };

  return {std::move(vertexList), std::move(indices)};
}
}
