#pragma once

#include "as/VertexList.hpp"

namespace tr {

/*
  Need to decide what differentiates this from a BufferCreateInfo.
  I guess BufferCreateInfo is vulkan specific, and BufferConfig abstracts that away
  Need to add like Flags and MemoryUsage and MemoryProperties abstracted flags here.
*/

enum class BufferType : uint8_t {
  Geometry = 0U,
  Arena,
  Fixed,
  Indirect
};

struct BufferConfig {
  size_t id = 0L;
  size_t size;
  BufferType bufferType;

  std::optional<VertexFormat> vertexFormat{};
  std::optional<size_t> stride{};

  auto operator==(const BufferConfig& other) const -> bool {
    return id == other.id && size == other.size && bufferType == other.bufferType &&
           vertexFormat == other.vertexFormat && stride == other.stride;
  }
};
}

template <>
struct std::hash<tr::BufferConfig> {
  auto operator()(const tr::BufferConfig& config) const -> size_t {
    size_t h = 0;

    auto hash_combine = [&h](const auto& value) {
      h ^= std::hash<std::decay_t<decltype(value)>>{}(value) + 0x9e3779b9 + (h << 6U) + (h >> 2U);
    };

    hash_combine(config.id);
    hash_combine(config.size);
    hash_combine(static_cast<uint8_t>(config.bufferType));

    if (config.vertexFormat.has_value()) {
      hash_combine(config.vertexFormat.value());
    }

    if (config.stride.has_value()) {
      hash_combine(config.stride.value());
    }

    return h;
  }
};
