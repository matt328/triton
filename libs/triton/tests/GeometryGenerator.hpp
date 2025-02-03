#pragma once

#include "geo/StaticGeometryData.hpp"

inline auto generateRandomVertices(size_t count) -> std::vector<as::StaticVertex> {
  std::vector<as::StaticVertex> vertices;
  vertices.reserve(count);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
  std::uniform_real_distribution<float> texDist(0.0f, 1.0f);

  for (size_t i = 0; i < count; ++i) {
    vertices.push_back({
        {posDist(gen), posDist(gen), posDist(gen)}, // Random position
        {texDist(gen), texDist(gen)}                // Random texture coordinate
    });
  }

  return vertices;
}

inline auto generateMesh(size_t vertexCount, size_t indexCount) -> tr::StaticGeometryData {
  auto vertices = generateRandomVertices(vertexCount);

  auto indices = std::vector<uint32_t>{};

  // Generate indices (assuming triangle list)
  indices.reserve(indexCount);

  for (size_t i = 0; i < indexCount; ++i) {
    indices.push_back(static_cast<uint32_t>(i));
  }

  return tr::StaticGeometryData(std::move(vertices), std::move(indices));
}
