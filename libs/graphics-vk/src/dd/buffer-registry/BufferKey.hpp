#pragma once

#include "as/VertexList.hpp"
#include "bk/Hash.hpp"

namespace tr {

struct GeometryExtraProfile {
  VertexFormat vertexFormat;
  size_t indexSize;

  auto operator==(const GeometryExtraProfile& other) const -> bool {
    return vertexFormat == other.vertexFormat && indexSize == other.indexSize;
  }
};

struct ObjectDataExtraProfile {
  size_t objectStride;

  auto operator==(const ObjectDataExtraProfile& other) const -> bool {
    return objectStride == other.objectStride;
  }
};

enum class BufferKind : uint8_t {
  GpuBufferRegion = 0,
  ObjectCount,
  ObjectData,
  ObjectDataIndex,
  IndirectCommand,
  Geometry,
  Material
};

enum class BufferUsage : uint8_t {
  Storage = 0,
  Indirect,
  GeometryVertex,
  GeometryIndex,
  ShaderDeviceAddress,
  CpuWritable
};

using ExtraBufferInfo = std::variant<std::monostate, GeometryExtraProfile, ObjectDataExtraProfile>;

struct BufferUsageProfile {
  BufferKind kind;
  std::vector<BufferUsage> usages;
  std::string debugName;
  size_t stride = 0;
  size_t maxElements = 0;
  ExtraBufferInfo extra = std::monostate{};

  auto operator==(const BufferUsageProfile& other) const -> bool {
    return kind == other.kind && vecEqual(usages, other.usages) && debugName == other.debugName &&
           stride == other.stride && maxElements == other.maxElements && extra == other.extra;
  }

  template <typename T>
  auto getExtraAs() const -> const T* {
    return std::get_if<T>(&extra);
  }
};

struct BufferInstanceKey {
  size_t drawContextId;
  uint8_t frameId;

  auto operator==(const BufferInstanceKey& other) const -> bool {
    return drawContextId == other.drawContextId && frameId == other.frameId;
  }
};

struct BufferKey {
  BufferUsageProfile profile;
  BufferInstanceKey instance;
  auto operator==(const BufferKey& other) const -> bool {
    return profile == other.profile && instance == other.instance;
  }
};

}

namespace std {
template <>
struct hash<tr::GeometryExtraProfile> {
  auto operator()(const tr::GeometryExtraProfile& v) const -> std::size_t {
    std::size_t h = 0;
    hash_combine(h, v.vertexFormat);
    hash_combine(h, v.indexSize);
    return h;
  }
};

template <>
struct hash<tr::ObjectDataExtraProfile> {
  auto operator()(const tr::ObjectDataExtraProfile& v) const -> std::size_t {
    return std::hash<size_t>{}(v.objectStride);
  }
};

template <>
struct hash<tr::ExtraBufferInfo> {
  auto operator()(const tr::ExtraBufferInfo& info) const -> std::size_t {
    return std::visit(
        [](auto&& val) -> std::size_t {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
          } else {
            return std::hash<T>{}(val);
          }
        },
        info);
  }
};

template <>
struct hash<tr::BufferUsageProfile> {
  auto operator()(const tr::BufferUsageProfile& v) const -> std::size_t {
    std::size_t h = 0;
    hash_combine(h, static_cast<uint8_t>(v.kind));
    for (const auto& usage : v.usages) {
      hash_combine(h, static_cast<uint8_t>(usage));
    }
    hash_combine(h, v.stride);
    hash_combine(h, v.maxElements);
    hash_combine(h, v.extra);
    return h;
  }
};

template <>
struct hash<tr::BufferInstanceKey> {
  auto operator()(const tr::BufferInstanceKey& v) const -> std::size_t {
    std::size_t h = 0;
    hash_combine(h, v.drawContextId);
    hash_combine(h, v.frameId);
    return h;
  }
};

template <>
struct hash<tr::BufferKey> {
  auto operator()(const tr::BufferKey& k) const -> std::size_t {
    std::size_t h = 0;
    hash_combine(h, k.profile);
    hash_combine(h, k.instance);
    return h;
  }
};
}
