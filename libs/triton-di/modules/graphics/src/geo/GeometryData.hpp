#pragma once

namespace tr {

class IGeometryData {
public:
  IGeometryData() = default;
  virtual ~IGeometryData() = default;

  IGeometryData(const IGeometryData&) = default;
  IGeometryData(IGeometryData&&) = delete;
  auto operator=(const IGeometryData&) -> IGeometryData& = default;
  auto operator=(IGeometryData&&) -> IGeometryData& = delete;

  [[nodiscard]] virtual auto getVertexDataSize() const -> size_t = 0;
  [[nodiscard]] virtual auto getIndexDataSize() const -> size_t = 0;
  [[nodiscard]] virtual auto getVertexData() const -> const void* = 0;
  [[nodiscard]] virtual auto getIndexData() const -> const void* = 0;
  [[nodiscard]] virtual auto getIndexCount() const -> size_t = 0;
  [[nodiscard]] virtual auto getVertexCount() const -> size_t = 0;
};

template <typename T>
struct GeometryData {
  std::vector<T> vertices;
  std::vector<uint32_t> indices;

  [[nodiscard]] auto vertexDataSize() const noexcept {
    return sizeof(vertices[0]) * vertices.size();
  }

  [[nodiscard]] auto indexDataSize() const noexcept {
    return sizeof(indices[0]) * indices.size();
  }
};

}
